#!/bin/bash

GREEN='\033[0;32m'
BLUE='\033[0;34m'
ORANGE='\033[0;33m'
NC='\033[0m' # No Color


cd raiz

echo -e "${BLUE} dentro de ${ORANGE}raiz${BLUE} hay: ${NC}"

ls

echo -e "${BLUE} verifico que en ${ORANGE}stat_c${BLUE} se haya guardado lo que había:${NC}"

cat stat_c

echo " ----------------- "

echo -e "${BLUE} dentro de ${ORANGE}subcarpeta${BLUE} hay: ${NC}"

cd subcarpeta

ls

echo " ----------------- "

echo -e "${BLUE} dentro de ${ORANGE}sub-subcarpeta${BLUE} hay: ${NC}"

cd sub-subcarpeta

ls

echo " ----------------- "

cd ../../..

make clean

umount raiz
