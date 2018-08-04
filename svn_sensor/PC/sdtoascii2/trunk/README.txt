README.txt
08/02/2018

Hack of sdtoascii, but for current logger format

Each CAN msg does not carry a time, therefore the time search
requires scanning through msgs until a CAN id for the msg carrying
the date/time is found.


