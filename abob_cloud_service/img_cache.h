#pragma once

#include "crypto.h"
#include "image.h"

struct Image *search(const struct Hash *h);
void search_destruct(struct Image *img);
