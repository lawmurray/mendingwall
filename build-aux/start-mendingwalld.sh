#!/usr/bin/env bash

HOST_XDG_DATA_DIRS=""
IFS=: read -ra host_data_dirs < <(flatpak-spawn --host sh -c 'echo "$XDG_DATA_DIRS"')
for dir in "${host_data_dirs[@]}"; do
  if [[ "$dir" == /usr/* ]]; then
    dir="/run/host${dir}"
  fi
  HOST_XDG_DATA_DIRS="${HOST_XDG_DATA_DIRS}:${dir}"
done
export XDG_DATA_DIRS="${HOST_XDG_DATA_DIRS:1}:${XDG_DATA_DIRS}"
export GSETTINGS_BACKEND=dconf
exec mendingwalld "$@"
