ARG cpprestsdk_version=latest
ARG boost_version=latest
ARG debian_tag=latest

FROM cpprestsdk:${cpprestsdk_version} as env_cpprestsdk

FROM build_env_boost:${boost_version} as build_env

# Take binaries and includes from the cpprestsdk environment
COPY --from=env_cpprestsdk /usr/local/lib /usr/local/lib
COPY --from=env_cpprestsdk /usr/local/include /usr/local/include

WORKDIR /app

# Argument for number of cores to use while building
ARG num_cores

COPY CMakeLists.txt ./mwe_service/CMakeLists.txt
COPY src ./mwe_service/src

RUN cd mwe_service \
  && mkdir build.release \
  && cd build.release \
  && cmake .. -DCMAKE_BUILD_TYPE=Release \
  && make -j ${num_cores}

FROM debian:${debian_tag}

WORKDIR /app

COPY --from=build_env /app/mwe_service/bin/ /app/bin
COPY --from=build_env /usr/local/lib /usr/local/lib

RUN apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y dos2unix \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/* \
  && ldconfig

COPY run_module.sh /run_module.sh
RUN dos2unix /run_module.sh

WORKDIR /app/bin

ENTRYPOINT ["/bin/bash", "/run_module.sh"]