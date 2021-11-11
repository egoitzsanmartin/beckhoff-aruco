# ArUco detector
Este programa implementa distintas funcionalidades con el objetivo de crear un programa que sea capaz de capturar la pose de un robot utilizando marcadores ArUco.

## REQUISITOS PREVIOS
Librería de Computer Vision de OpenCV instalada: https://sourceforge.net/projects/opencvlibrary/files/4.4.0/opencv-4.4.0-vc14_vc15.exe/download

Módulos extra de OpenCV (renombrar a opencv_contrib-master si es necesario): https://github.com/opencv/opencv_contrib

Librerías para la manipulación de cámaras mvBlueCOUGAR de Matrix Vision: https://www.matrix-vision.com/en/downloads/drivers-software/mvbluecougar-gigabit-ethernet-dual-gigabit-ethernet-10gige-ethernet/windows-7-8-1-10

Entorno del software de control de TwinCAT 3: https://www.beckhoff.com/es-es/support/download-finder/software-and-tools/
#### NOTA: Instalar ambos en la raiz del disco principal (C:\)

## FUNCIONALIDADES

### Captura de imagen multi-cámara con Matrix Vision

Utiliza funciones de las librerías de **Matrix Vision** y **OpenCV** para **capturar** e incluso **guardar en disco** imágenes capturadas por todas las cámaras conectadas mediante un **trigger** digital externo. Las imágenes se guardan en una carpeta llamada "img" + la id de la cámara a la que pertenezcan las imágenes, en formato bitmap. También permite cambiar la configuración de estas cámaras, ya sea la **ganancia**, **exposición** o **framerate** de esta. Mediante unas variables que se encuentran en la parte superior del archivo "programa", se puede decidir si guardar la imagen o no, si mostrar la imagen en pantalla o no, si escribir la posición de los marcadores en ficheros o no, si detectar marcadores o no, determinar el tamño de los marcadores, modificar la ruta donde se guardan los archivos y cambiar el formato de imaggen.
  
### Detección de marcadores Aruco

Utilizando funciones de **OpenCV** el programa es capaz de **detectar marcadores ArUco** a partir de las imágenes capturadas por las cámaras de Matrix Vision. Además, si los parametros de calibración de cámara han sido introducidos, el programa es capaz de estimar el **cambio de posición y rotación** de estos marcadores, guardándolos en una carpeta de nombre "poses" y dentro en un fichero de texto con distinto nombre dependiendo de la id de la cámara.
  
### Comunicación con PLC de Beckhoff mediante ADS

Posee funciones de las librerías de TwinCAT ADS para la comunicación con el PLC de un ordenador Beckhoff.
  
### Comunicación con robot mediante UDP

Esta funcionalidad no se utiliza, ya que ha sido reemplazada por la comunicación con ADS y el PLC, la cuál está todavía en desarrollo. La implementación de UDP consiste en tener un puerto activo y escuchando para que el robot envíe un XML con la información de su pose y mediante un parseador, escriba en fichero estos datos.
