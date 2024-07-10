#!/bin/bash


make clean

make build

if [[ "$1" == "p" ]]; then
  echo "Prueba de persistencia"
else
  # Si la carpeta raiz existe, la borramos
  if [[ -d raiz ]]; then
    rm -r raiz
  fi

  # Creamos la carpeta raiz
  mkdir -p raiz

  # Si el archivo filesystem.fisopfs existe, lo borramos
  if [[ -e filesystem.fisopfs ]]; then
    rm filesystem.fisopfs
  fi
fi

./fisopfs -f raiz
