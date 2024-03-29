#include "img_db.h"
#include "common.h"

#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

sqlite3 *db;

void init_db() {
  sqlite3_open("images.db", &db);

  if (db == NULL) {
    die("Failed to open DB %s", sqlite3_errmsg(db));
  }

  sqlite3_stmt *stmt;
  int res = sqlite3_prepare_v2(
      db, "CREATE TABLE IF NOT EXISTS Images (hash TEXT PRIMARY KEY, img BLOB)",
      -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    die("Cannot create table %s", sqlite3_errmsg(db));
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    printf("[ERROR] creating table: %s\n", sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
}

void close_db() { sqlite3_close(db); }

void add(const struct Hash *h, const struct Image *img) {
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, "INSERT INTO Images(hash, img) VALUES (?, ?)", -1,
                         &stmt, NULL) != SQLITE_OK) {
    printf("[ERROR] Data cannot be prepared for insertion %s\n",
           sqlite3_errmsg(db));
    return;
  }

  if (sqlite3_bind_text(stmt, 1, h->data, -1, SQLITE_STATIC) != SQLITE_OK ||
      sqlite3_bind_blob(stmt, 2, &img->len, sizeof(img->len) + img->len, SQLITE_STATIC) !=
          SQLITE_OK) {
    printf("[ERROR] cannot insert data: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return;
  }

  int status = 0;
  while ((status = sqlite3_step(stmt)) == SQLITE_BUSY) sleep(random() % 3);
  if (status != SQLITE_DONE) {
    printf("[ERROR] cannot insert data: [%d] %s\n", status, sqlite3_errmsg(db));
  }
  sqlite3_finalize(stmt);
}

struct Image *db_search(const struct Hash *h) {
  printf("[INFO] starting img search in db\n");
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, "SELECT img FROM Images WHERE hash LIKE ?", -1,
                         &stmt, NULL) != SQLITE_OK) {
    printf("[ERROR] Data cannot be prepared for insertion %s\n",
           sqlite3_errmsg(db));
    return NULL;
  }

  if (sqlite3_bind_text(stmt, 1, h->data, HEX_HASH_SIZE, SQLITE_STATIC) != SQLITE_OK) {
    printf("[ERROR] cannot bind data: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return NULL;
  }

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    printf("[WARN] no image found\n");
    sqlite3_finalize(stmt);
    return NULL;
  }

  size_t *ptr = (size_t *)sqlite3_column_blob(stmt, 0);
  size_t len = *ptr;

  struct Image *img = new_img(len);
  img->len = len;
  memcpy(img->data, ptr + 1, img->len);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    // printf("[WARN] that should happen, sqlite seems broken\n");
  }

  sqlite3_finalize(stmt);
  return img;
}
