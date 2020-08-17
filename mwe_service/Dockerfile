ARG debian_tag=latest
FROM debian:${debian_tag}

RUN apt-get update && apt-get upgrade -y \
 && apt-get install -f -y build-essential dos2unix cmake \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Argument for number of cores to use while building
ARG num_cores

COPY CMakeLists.txt ./CMakeLists.txt
COPY src ./src

RUN mkdir build.release \
  && cd build.release \
  && cmake .. -DCMAKE_BUILD_TYPE=Release \
  && make -j ${num_cores}

COPY run_module.sh /run_module.sh
RUN dos2unix /run_module.sh

WORKDIR /app/bin

ENTRYPOINT ["/bin/bash", "/run_module.sh"]
