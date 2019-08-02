#!/bin/bash

echo "Script para ejecutar la prueba de stress en el FileSystem"

if [ ! -d Config ];
then
echo "se crea la carpeta de Config"
mkdir Config
fi

cd Config/

if [ !  -f LFS_CONFIG.txt ];
then
echo "se borra el archivo anterior de config"
rm -r  LFS_CONFIG.txt
fi

cp ../../CONFIGS_SCRIPTS/PRUEBA_STRESS/LFS_CONFIG.txt ./
echo "Config de stress copiado"

cd ..

if [ ! -d Metadata ];
then
echo "se crea la carpeta de Metadata"
mkdir Metadata
fi

cd Metadata/

if [ !  -f Metadata.txt ];
then
echo "se borra el archivo anterior de Metadata"
rm -r  Metadata
fi

cp ../../METADATAS_SCRIPTS/PRUEBA_STRESS/METADATA ./
echo "Config de stress copiado"

cd ..

if [ !  -x scriptInicial.sh ];
then
echo "se le da permisos de ejecucion al scriptInicial.sh"
chmod +x scriptInicial.sh
fi

./scriptInicial.sh

