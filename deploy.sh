#!/bin/bash

MOUNTPOINT="/opt/dev"

make clean -C $MOUNTPOINT &&                     \
  make check_loader -C $MOUNTPOINT &&            \
  rm -f /tmp/abob_cloud &&                       \
  cp $MOUNTPOINT/bin/real_loader ~/abob_loader
