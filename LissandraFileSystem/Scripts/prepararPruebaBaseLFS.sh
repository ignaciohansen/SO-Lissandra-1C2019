#!/bin/bash

IP_LFS=127.0.0.1

if [ $# -eq 1 ];
then
IP_LFS=$1
fi

if [ $# -gt 1 ];
then
echo $#
echo "El unico parametro habiitado es la ip"
exit 1
fi

echo "Script para ejecutar la prueba de base en el FileSystem"

cd ..

if [ ! -d Config ];
then
echo "se crea la carpeta de Config"
mkdir Config
fi

cd Config/

if [  -f LFS_CONFIG.txt ];
then
echo "se borra el archivo anterior de config"
rm -r  LFS_CONFIG.txt
fi

cp ../../CONFIGS_SCRIPTS/PRUEBA_BASE/LFS_CONFIG.txt ./
echo "Config base copiado"

sed -i -e "s/ip/$IP_LFS/" LFS_CONFIG.txt

cd ..

if [ ! -d Metadata ];
then
echo "se crea la carpeta de Metadata"
mkdir Metadata
fi

cd Metadata/

if [  -f Metadata ];
then
echo "se borra el archivo anterior de Metadata"
rm -r  Metadata
fi

cp ../../METADATAS_SCRIPTS/PRUEBA_BASE/Metadata ./
echo "Metadata base copiada"

cd ../Scripts

if [ !  -x scriptInicial.sh ];
then
echo "se le da permisos de ejecucion al scriptInicial.sh"
chmod +x scriptInicial.sh
fi

./scriptInicial.sh