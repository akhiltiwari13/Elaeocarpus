#!/bin/bash

# Create logs directory
mkdir -p logs

# Set proper permissions for multicast
sudo setcap cap_net_bind_service,cap_net_raw=+ep ./mcx_receiver

# Create log directory with proper permissions
chmod 755 logs

echo "Setup complete. You can now run the MCX receiver."
