#!/bin/bash

IP_MEM3=127.0.0.1

if [ $# -eq 1 ];
then
IP_MEM3=$1
fi

echo "Script para ejecutar la prueba de kernel en el kernel"

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

cp ../../CONFIGS_SCRIPTS/PRUEBA_KERNEL/KERNEL.txt ./
echo "Config de kernel copiado"

sed -i -e "s/mem3/$IP_MEM3/" KERNEL.txt
