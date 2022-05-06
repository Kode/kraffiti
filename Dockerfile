FROM alpine
RUN apk add build-base git linux-headers
ADD . /kraffiti
RUN git clone --depth 1 https://github.com/Kode/KincTools_linux_x64.git
WORKDIR "/kraffiti"
RUN /KincTools_linux_x64/kmake --compile
CMD cp /kraffiti/build/Release/kraffiti /workdir/kraffiti_linux_x64
