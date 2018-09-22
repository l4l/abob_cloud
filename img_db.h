#pragma once

#include "crypto.h"
#include "image.h"

void init_db();
void close_db();

void add(const struct Hash *h, const struct Image *img);
struct Image *search(const struct Hash *h);
