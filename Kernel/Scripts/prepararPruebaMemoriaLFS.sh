#!/bin/bash

echo "Script para ejecutar la prueba de memoria en el kernel"

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

cp ../../CONFIGS_SCRIPTS/PRUEBA_MEMORIA/KERNEL.txt ./
echo "Config de memoria copiado"

