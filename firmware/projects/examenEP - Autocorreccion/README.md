#Examen 

El sistema está compuesto por un acelerómetro analógico montado sobre el casco y un
HC-SR04 ubicado en la parte trasera de la bicicleta.
Por un lado se debe detectar la presencia de vehículos detrás de la bicicleta. Para ello
se utiliza el sensor de ultrasonido. Se indicará mediante los leds de la placa la distancia
de los vehículos a la bicicleta, encendiéndose de la siguiente manera:
● Led verde para distancias mayores a 5 metros
● Led verde y amarillo para distancias entre 5 y 3 metros
● Led verde, amariilo y rojo para distancias menores a 3 metros.
La medición con el HC-SR04 deberá realizarse dos veces por segundo. Además de los
mensajes se deberá activar una alarma sonora mediante un buzzer activo. Este
dispositivo suena cuando uno de los GPIOs de la placa se pone en alto y se apaga
cuando se pone en bajo. La alarma sonará con una frecuencia de 1 segundo en el caso
de precaución y cada 0.5 segundos en el caso de peligro.
Además se deberá enviar una notificación a una Aplicación corriendo en un
Smartphone. Esta notificación se envía utilizando un módulo bluetooth conectado al
segundo puerto serie de la placa ESP-EDU. Se enviarán los siguientes mensajes:
● “Precaución, vehículo cerca”, para distancias entre 3 a 5 metros
● “Peligro, vehículo cerca”, para distancias menores a 3 metros
El acelerómetro ubicado en el casco tiene la finalidad de detectar golpes o caídas. Se
trata de un acelerómetro analógico triaxial (3 canales), muestreado a 100Hz, con una
salida de 1.65 V para 0 G y una sensibilidad de 0.3V/G:
Si la sumatoria (escalar) de la aceleración en los tres ejes supera los 4G se deberá
enviar el siguiente mensaje a la aplicación:
● “Caída detectada”