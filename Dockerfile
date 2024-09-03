FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y python3 python3-pip build-essential gcc g++ make cmake

RUN rm -rf /usr/bin/python && \
    rm -rf /usr/bin/pip

RUN ln -s /usr/bin/python3 /usr/bin/python && \
    ln -s /usr/bin/pip3 /usr/bin/pip

WORKDIR /usr/src/app

COPY . /usr/src/app

CMD ["python", "setup.py", "sdist", "bdist_wheel"]
