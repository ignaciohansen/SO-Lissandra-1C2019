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

echo "Script para ejecutar la prueba de memoria en el FileSystem"

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

cp  ../../CONFIGS_SCRIPTS/PRUEBA_MEMORIA/LFS_CONFIG.txt ./
echo "Config de memoria copiado"

sed -i -e "s/ip/$IP_LFS/" LFS_CONFIG.txt

if [ ! -d /home/utnso/lfs-prueba-memoria ];
then
mkdir /home/utnso/lfs-prueba-memoria
else
rm -r /home/utnso/lfs-prueba-memoria/*
fi

cd /home/utnso/lfs-prueba-memoria

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

cp /home/utnso/tp-2019-1c-mi_ultimo_segundo_tp/METADATAS_SCRIPTS/PRUEBA_MEMORIA/Metadata ./
echo "Metadata de memoria copiada"

cd /home/utnso/tp-2019-1c-mi_ultimo_segundo_tp/LissandraFileSystem/Scripts
