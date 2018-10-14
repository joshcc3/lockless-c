FROM ubuntu
RUN apt-get update && apt-get install -y gdb && apt-get install -y gcc && apt-get install -y make
