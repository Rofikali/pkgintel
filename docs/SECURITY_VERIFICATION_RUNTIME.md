# Dedicated Security Verification Runtime

## Purpose

`pkgintel-security` is the repository-owned Docker Compose runtime for genuine Linux filesystem/VFS security verification. It exists so privileged kernel operations required by the hostile-filesystem tests are isolated from the normal least-privileged development container.

This is a **verification runtime**, not a production deployment image.

## Canonical repository definition

The runtime is defined by:

- `Dockerfile`
- `docker-compose.yml`
- Compose service: `pkgintel-security`
- Compose profile: `security`
- container name: `pkgintel-security-verify`

The service extends the normal `pkgintel` service instead of duplicating its build, mount, environment, and working-directory configuration.

## Runtime security boundary

Normal development:
- service: `pkgintel`
- user: `developer` (UID 1001 in the image)
- `SYS_PTRACE` capability
- `seccomp:unconfined`
- not privileged

Security verification:
- service: `pkgintel-security`
- `privileged: true`
- `user: root`
- Compose profile: `security`
- inherits the repository workspace bind mount
- intended only for explicit genuine filesystem/VFS verification

The privilege escalation is deliberately scoped to the verification service. Do not make the normal development service privileged merely to make the mount test pass.

## Verified image identity

The Docker Desktop host inspection produced:

| Property | Observed value |
|---|---|
| Repository tag | `pkgintel-pkgintel-security:latest` |
| Image ID | `sha256:5327be1fff1b9b00fdc578f08b6a4d895117df7e12b7afdb4c050546e82ca6e4` |
| RepoDigest | none; image is local/not registry-published |
| Created | `2026-10-06T05:39:58.271215156Z` |
| OS | `linux` |
| Architecture | `amd64` |
| Entrypoint | null |
| Command | `["bash"]` |
| Compose project | `pkgintel` |
| Compose service label | `pkgintel-security` |
| Compose version label | `2.34.0` |
| OCI version label | `24.04` |

The immutable **image ID** is the release-evidence identity. The `latest` tag is only a mutable convenience alias.

Because `RepoDigests=[]`, this image must not be described as a registry-pinned image. If the runtime is later published to a registry, record the registry digest separately.

## Image/source provenance distinction

The image is a toolchain/runtime artifact. The repository workspace is bind-mounted into the container:

```text
./:/workspace/pkgintel
```

Therefore the source code exercised by the security test is determined by the mounted Git checkout, not by copying the repository into the image layer.

This distinction is critical:

```text
Docker image identity
    !=
pkgintel source SHA
```

Both must be recorded for reproducible security evidence.

The current image was created on 2026-10-06. Its inspection does **not** by itself prove which Git SHA was checked out when the image was built. That is not a blocker to using it as a runtime/toolchain artifact because the Compose workspace is mounted from the current checkout, but every security test result must still record the exact source SHA at test execution time.

## Runtime qualification

The repository's genuine filesystem security evidence requires the runtime to demonstrate that the kernel primitive is actually available.

The required qualification evidence includes, as applicable:
- effective UID/GID;
- effective/bounding capabilities;
- seccomp mode/filter state;
- user-namespace identity mapping;
- mount namespace identity;
- successful real `mount --bind` probe;
- successful execution of the unmodified pkgintel hostile-filesystem test.

A successful Docker image build is not security evidence.

A privileged container that cannot perform the required mount operation is not sufficient evidence.

A simulated bind mount or ordinary directory fixture is not equivalent to a real VFS mount-boundary test.

## Verification command contract

The canonical strict build/test mode is:

```bash
cmake -S . -B build-security \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPKGINTEL_BUILD_TESTS=ON \
  -DPKGINTEL_REQUIRE_PRIVILEGED_SECURITY_TESTS=ON

cmake --build build-security -j"$(nproc)"

sudo ctest --test-dir build-security \
  -R 'pkgintel[.]security[.]mounts' \
  --output-on-failure
```

For full security-runtime verification:

```bash
ctest --test-dir build-security --output-on-failure
```

The strict security option is intentional: if the runtime cannot perform the privileged operation, the test must fail rather than silently return the ordinary unavailability skip.

## Evidence classification

The runtime itself establishes:

**Source:** repository-defined security service exists.

The image inspection establishes:

**Runtime artifact identity:** exact local image ID, OS, architecture, and Compose labels are known.

Only the returned runtime/test output establishes:

**Real runtime evidence:** actual Linux VFS behavior was exercised successfully.

Do not promote image identity into security proof.

## Rebuild/requalification policy

Do not rebuild this image merely because time passed.

Rebuild or requalify when material inputs change, including:
- `Dockerfile`;
- `docker-compose.yml` security configuration;
- base image;
- installed toolchain/security-relevant packages;
- Docker Desktop/runtime behavior;
- host kernel/platform;
- security capabilities or namespace model;
- verification command or security test;
- source/build configuration when it affects the tested property.

When only documentation changes and the runtime configuration and source security surface are unchanged, retain prior evidence and record applicability rather than creating duplicate verification work.

## Security and management rationale

### Principal Security Engineering

The privileged boundary is explicit and isolated. The test is fail-closed with respect to unavailable mount capability. The image identity and runtime configuration are independently recorded from the source SHA and test result.

### Staff/Principal Software Engineering

The Compose `extends` relationship prevents configuration drift between development and security runtimes while preserving a separate privilege boundary.

### CA/Finance

One dedicated verification image/service is lower-maintenance than maintaining multiple duplicated security environments. Rebuilding is evidence-driven, reducing unnecessary compute/time cost without weakening release assurance.

### MBA/Management/Product

The service is a release-verification dependency, not a customer-facing runtime. Its scope should remain narrow: prove the Linux filesystem security property required by pkgintel and avoid turning the development environment into a privileged default.

## Final rule

```text
Know the source SHA.
Know the image ID.
Know the runtime configuration.
Know the kernel/platform.
Know the exact command.
Know the observed result.
Do not confuse any one of these with the others.
```