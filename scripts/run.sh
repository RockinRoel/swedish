#!/bin/sh

dir=$(cd -P -- "$(dirname -- "$0")" && pwd -P)
cd ${dir}
cd ..
exec ./bin/swedish.wt --docroot "docroot;/css,/js,/puzzles,/resources" --approot approot -c approot/wt_config.xml --http-listen 0.0.0.0:1234 --resources-dir /home/roel/projects/wt/git/wt/resources
