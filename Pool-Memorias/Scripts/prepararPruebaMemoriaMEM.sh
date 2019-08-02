#!/bin/bash

if [ ! $# = 1 ];
then
echo "Estaba esperando 1 parametro para el numero de memoria"
exit 1
fi

echo "Script para ejecutar la prueba de memoria en la memoria $1"

cd ..

if [ ! -d Config ];
then
echo "se crea la carpeta de Config"
mkdir Config
fi

cd Config/

if [  -f MEMORIA.txt ];
then
echo "se borra el archivo anterior de config"
rm -r  MEMORIA.txt
fi

cp ../../CONFIGS_SCRIPTS/PRUEBA_BASE/MEM$1_CONFIG.txt ./
echo "Config memoria copiado"

mv MEM$1_CONFIG.txt MEMORIA.txt