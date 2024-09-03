FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y python3 python3-pip python3-setuptools python3-wheel build-essential

RUN rm -rf /usr/bin/python && \
    rm -rf /usr/bin/pip

RUN ln -s /usr/bin/python3 /usr/bin/python && \
    ln -s /usr/bin/pip3 /usr/bin/pip

WORKDIR /usr/src/app

COPY . /usr/src/app

RUN pip install --upgrade pip && \
    pip install --no-cache-dir cmake wheel setuptools pybind11

CMD ["python", "setup.py", "sdist", "bdist_wheel"]
