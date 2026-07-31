# Botan import record

- Upstream: Botan 3.12.0
- Source commit: `45d6f286c320b2f2efd5373d195ec88c367e3071`
- Source archive: `https://github.com/randombit/botan/archive/refs/tags/3.12.0.tar.gz`
- Archive SHA-256: `cf152f47723876a7b8544925fc37183089933b95b9b30f41e79ab2f263ab7995`
- License: BSD 2-Clause; the complete text is in `license.txt`.

`app/src/main/native/vc_core/botan/` contains three generated, minimized
amalgamations for the supported Android ABIs. They were generated from this
exact source tree with Botan's `configure.py`, `--os=android`, the ABI CPU,
and only these modules: AES, Serpent, Twofish, Camellia, Kuznyechik, XTS,
PBKDF2, HMAC, SHA-2, BLAKE2s, Whirlpool, Streebog and Argon2.

The generated sources are linked statically into `vc_core`; the application
must not download or dynamically load a Botan library.
