# Stage 1: build
FROM alpine:3.22.1 AS build

RUN apk add --no-cache \
    build-base \
    cmake \
    git

WORKDIR /app
COPY . /app
RUN mkdir -p bin && cd bin && cmake .. && make && ls -l

# Stage 2: final image
FROM alpine:3.22.1

WORKDIR /app

RUN apk add --no-cache \
        libstdc++ \
        libgcc \
        sqlite-libs \
        openssl \
        libmosquitto \
        ca-certificates \
    && update-ca-certificates || true

COPY --from=build /app/bin/auth.service /app/auth.service

CMD ["./auth.service"]
