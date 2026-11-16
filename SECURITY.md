# Security & Responsible Use Policy

## Intended Use

Project RAILROAD is an **educational ICS/SCADA cybersecurity testbed**. The offensive
tooling in `red-team/` (the "IRON SPIDER" attack console and its Modbus scripts) was
written to test **the author's own physically isolated laboratory network**, built from
hobby hardware (Raspberry Pi, ESP32 microcontrollers, a model railroad) on a dedicated
WiFi segment with no connection to production, utility, or public infrastructure.

## Authorized Use Only

Modbus TCP has **no built-in authentication or encryption**. The scripts here can read
and write coils/registers on any reachable Modbus device. Using them against systems you
do **not** own, or for which you do **not** hold explicit written authorization, is
illegal in most jurisdictions and is **not** condoned by this project.

**Before running anything in `red-team/`:**
- Confirm every target is on your own isolated lab network.
- Never point the tooling at real railway, utility, manufacturing, or any production OT.
- Keep the lab air-gapped or firewalled from the internet and other networks.

## No Secrets in the Repository

- WiFi credentials, API keys, and tokens must **never** be committed. Firmware reads
  credentials from a local, git-ignored `config.h` (see `config.example.h`).
- Lab IP addresses in the code are examples; change them to match your own segment.
- If you discover a committed secret, rotate it immediately and scrub history.

## Reporting

This is a personal/academic project, not a maintained product. If you find a security
issue in the materials here, please open an issue (omit any sensitive details).

## Disclaimer

The materials are provided for learning and defensive research. The author accepts no
liability for misuse. **You are responsible for how you use this code.**
