# Base image for C++ services with libpqxx
FROM ubuntu:24.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libssl-dev \
    libpq-dev \
    pkg-config \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Build and install libpqxx 7.9.2
WORKDIR /tmp
RUN wget https://github.com/jtv/libpqxx/archive/refs/tags/7.9.2.tar.gz && \
    tar -xzf 7.9.2.tar.gz && \
    cd libpqxx-7.9.2 && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/*

# Copy source code
WORKDIR /app
COPY . .

# Build the project
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Runtime image - smaller, only runtime dependencies
FROM ubuntu:24.04

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libpq5 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Copy libpqxx from builder
COPY --from=builder /usr/local/lib/libpqxx* /usr/local/lib/
COPY --from=builder /usr/local/include/pqxx /usr/local/include/pqxx

# Copy built binaries
COPY --from=builder /app/build/api-gateway/api-gateway /usr/local/bin/
COPY --from=builder /app/build/borrower-service/borrower-service /usr/local/bin/
COPY --from=builder /app/build/risk-assessment-service/risk-assessment-service /usr/local/bin/
COPY --from=builder /app/build/recovery-strategy-service/recovery-strategy-service /usr/local/bin/
COPY --from=builder /app/build/communication-service/communication-service /usr/local/bin/

# Update library cache
RUN ldconfig

# Default user
RUN useradd -m sdrs
USER sdrs

# Default command
CMD ["/bin/bash"]
