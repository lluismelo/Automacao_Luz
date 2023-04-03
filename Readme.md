Esse é o desenvolvimento de um projeto pessoal para automatizar o controle da iluminação de um quarto.
O controle poderá ser feito através de um sensor touch capacitivo e também através de comandos de voz com a Alexa.

Componentes:
1 x ESP 8266
1 x módulo relé
1 x sensor Touch
1 x fonte de alimentação 5v
1 x Regulador de tensão 3v3


Infos:

Para que seja possível fazer atualizações via OTA, é utilizada a biblioteca ArduinoOTA. No entando, ao adicionar a biblioteca pelo platformio, alguns parâmetros não eram identificados. Para resolver o problema, a biblioteca foi adicionada manualmente na pasta do projeto. A bibliote original está disponível no link: <https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA>