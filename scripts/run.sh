#!/bin/sh

dir="$(cd -P -- "$(dirname -- "$0")" && pwd -P)"
cd "${dir}" || exit 1
cd .. || exit 1
exec ./bin/swedish.wt \
  --docroot "docroot;/css,/js,/puzzles,/resources" \
  --approot approot \
  -c approot/wt_config.xml \
  --http-listen 0.0.0.0:1234 \
  --resources-dir /home/roel/project/wt/git/wt4/install-dir/static/share/Wt/resources
