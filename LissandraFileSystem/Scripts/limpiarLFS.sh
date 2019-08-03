#!/bin/bash

echo "Script para dejar el FileSystem listo para ejecutar"

cd ..

if [ -d Tables ];
then
	echo "Elimino carpeta Tables"
	rm -r Tables
fi

if [ -d Bloques ];
then
	echo "Elimino carpeta Bloques"
	rm -r Bloques
fi

cd Metadata/ 

if [ -f Bitmap.bin ];
then
	echo "Elimino archivo bitmap"
	rm -r Bitmap.bin
fi

