#!/bin/bash

echo "Script para ejecutar la prueba de stress en el kernel"

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
rm -r KERNEL.txt
fi

cp ../../CONFIGS_SCRIPTS/PRUEBA_STRESS/KERNEL.txt ./
echo "Config de stress copiado"

