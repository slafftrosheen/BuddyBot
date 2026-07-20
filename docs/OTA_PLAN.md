# BuddyBot OTA (Over-The-Air) Updates Plan

This document outlines the safety and security requirements for implementing OTA firmware updates on BuddyBot in the future.

## Principles

1. **Disabled by Default:** OTA must be an opt-in feature, disabled by default in standard builds.
2. **Physical Confirmation Required:** To prevent unauthorized or accidental flashing, a user must physically interact with BuddyBot (e.g., pressing a specific button combination) to accept an incoming OTA transfer.
3. **Hardware Safety:**
   - OTA updates must ONLY be allowed while motors are fully disarmed.
   - Any active Wi-Fi controller must be disconnected and their session token revoked before the upload begins.
4. **Firmware Integrity:**
   - Firmware image validation is mandatory.
   - Strict size limits must be enforced.
   - Implementation should use SHA-256 and signature verification strategies to ensure only valid builds are applied.
5. **Recovery:** The system must implement a reliable rollback or dual-bank partitioning scheme so that a failed update does not brick the robot.
6. **Network Exposure:**
   - There will be NO unauthenticated browser endpoints exposed for uploading firmware.
   - There will be NO OTA over the Internet (e.g. via cloud platforms) without a separately designed and audited authentication/security review. OTA is strictly for local, trusted network execution.
