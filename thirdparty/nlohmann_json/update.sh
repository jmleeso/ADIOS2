#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="nlohmann_json/single_include"
readonly ownership="JSON For Modern C++ Upstream <robot@adios2>"
readonly subtree="thirdparty/nlohmann_json/nlohmann_json/src/single_include/upstream"
readonly repo="https://github.com/nlohmann/json.git"
readonly tag="master"
readonly shortlog="true"
readonly paths="
  nlohmann/json.hpp
"

extract_source () {
    git_archive
}

. "${BASH_SOURCE%/*}/../update-common.sh"
