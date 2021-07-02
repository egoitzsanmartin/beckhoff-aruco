Este programa implementa distintas funcionalidades:

1.- CAPTURA DE IMÁGEN MULTI-CÁMARA CON MATRIX VISION

  Utiliza funciones de las librerías de Matrix Vision y OpenCV para capturar e incluso guardar en disco imagenes capturadas por todas las cámaras conectadas mediante
  un trigger. También permite cambiar la configuración de estas cámaras.
  
2.- DETECCIÓN DE MARCADORES ARUCO

  Utilizando funciones de OpenCV el programa es capaz de detectar marcadores ArUco a partir de las imágenes capturadas por las cámaras de Matrix Vision. Además,
  si los parametros de calibración de cámara han sido introducidos, el programa es capaz de estimar el cambio de posiciónn y rotación de estos marcadores.
  
3.- COMUNICACIÓN CON PLC DE BECKHOFF MEDIANTE ADS

  Posee funciones de las librerías de TwinCAT ADS para la comunicación con el PLC de un ordenador Beckhoff.
