FROM ubuntu:22.04 as builder
RUN apt-get update && apt-get install -y  build-essential cmake git python3 python3-pip ninja

# Install Conan
RUN pip3 install conan

# Create work directory
WORKDIR /elaeocarpus

# Copy source
COPY . .

# Configure and build
RUN mkdir build && cd build && \
  conan install .. --build=missing && \
  cmake .. -G ninja -DCMAKE_BUILD_TYPE=Debug && \
  cmake --build . --target trading_system

# Final stage
FROM ubuntu:22.04 as runner
WORKDIR /elaeocarpus

# Copy the compiled binary from the builder
COPY --from=builder /elaeocarpus/build/Elaeocarpus /usr/local/bin/Elaeocarpus

# Set default entrypoint
ENTRYPOINT ["/usr/local/bin/Elaeocarpus"]
