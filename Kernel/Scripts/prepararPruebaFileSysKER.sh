#!/bin/bash

IP_MEM1=127.0.0.1

if [ $# -eq 1 ];
then
IP_MEM1=$1
fi

echo "Script para ejecutar la prueba de LFS en el kernel"

cd ..

if [ ! -d Config ];
then
echo "se crea la carpeta de Config"
mkdir Config
fi

cd Config/

if [  -f KERNEL.txt ];
then
echo "se borra el archivo anterior de config"
rm -r  KERNEL.txt
fi

cp ../../CONFIGS_SCRIPTS/PRUEBA_LFS/KERNEL.txt ./
echo "Config de lfs copiado"

sed -i -e "s/mem1/$IP_MEM1/" KERNEL.txt