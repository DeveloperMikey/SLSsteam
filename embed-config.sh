#!/bin/bash

CONFIG="$(cat "./res/config.yaml")"

echo "#pragma once

constexpr static const char* defaultConfig = R\"($CONFIG)\";" > src/config_default.hpp
