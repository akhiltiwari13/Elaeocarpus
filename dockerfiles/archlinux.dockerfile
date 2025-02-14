FROM archlinux:latest

# Install dependencies (replace with your actual dependencies)
RUN pacman -Syu --noconfirm && pacman -S --noconfirm clang make git neovim cmake ccls clangd stow 

# Copy your SDK source code
COPY . /app

WORKDIR /app

# Build your SDK
RUN mkdir build && cd build && cmake .. && make -j$(nproc)

# Install your SDK (optional)
RUN make install

# Create a directory for your binaries
RUN mkdir /binaries

# Copy the binaries to the binaries directory
COPY build/your_binary /binaries
# ... copy other binaries

# Set the entrypoint for running the binaries (if needed)
# ENTRYPOINT ["/binaries/your_binary"]
