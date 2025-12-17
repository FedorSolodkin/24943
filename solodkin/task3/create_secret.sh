#!/bin/bash
echo "This is secret content" > secret.txt
chmod 600 secret.txt
echo "Secret file created with permissions:"
ls -la secret.txt