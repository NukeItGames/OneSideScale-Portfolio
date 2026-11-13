# Publication audit

Reviewed November 13, 2026. This is a bounded source, history, asset, and
dependency review; secret scanning cannot prove the absence of every possible
credential or vulnerability.

## Scope and source isolation

Reviewed all eight source commits, all 41 unique historical file blobs, the
current 16-file tree, build rules, descriptor, gizmo/mode/toolbar/input code,
transaction/bounds logic, and all five automation test definitions. History
contains only one branch and no tags. All recorded commit authors use the owner's
public GitHub noreply identity. No submodules or LFS pointers were identified.

Preparation used a read-only mirror and a tracked-file archive extracted into a
separate directory. The mirror has a disabled push URL. No upstream mutation,
source push, visibility change, or private history import is part of this work.
The public copy starts with a fresh Git root using the owner's public noreply
identity. The private source commit identifiers and original remote are not
included in its Git history.

All **12 files beneath Source/**, including build rules and the five Unreal
tests, match the reviewed source snapshot byte-for-byte. Publication changes are
documentation, descriptor attribution/public URLs, license, icon, ignore rules,
engine-independent tooling, and CI. The descriptor still declares exactly one
Editor module with no runtime module, content, or plugin dependency.

## Confidentiality and source security

Gitleaks **8.30.1** was obtained from the official release. The downloaded Linux
archive matched the release's published SHA-256:
`551f6fc83ea457d62a0d98237cbad105af8d557003051f41f3e7ca7b3f2470eb`.
The history scan, current source snapshot scan, and explicit scan of every unique
historical blob found **zero leaks**. The explicit blob scan covers initial
content and files deleted or replaced later, independently of diff traversal.
The final public snapshot and fresh history are scanned before pushing.

Supplemental review found a personal Windows user-directory path and a private
game project/target in historical README build commands. Neither is required by
the plugin. The public README uses user-supplied paths and a standalone host.
There are no remaining machine-specific paths, private repository links,
environment files, keys, certificates, generated binaries, symlinks, user data,
or private game dependencies in the publication snapshot. Commit email uses the
public noreply address, not the workstation's configured personal email.

No plugin network requests, process execution, telemetry, credentials, custom
file deserialization, or external package fetching was identified. Native editor
code shares Unreal's process privileges. Blueprint subclasses can run their own
construction scripts during editor move notifications. Development automation
temporarily changes the editor selection/mode and transaction buffer; use the
disposable host rather than a valuable working level.

## Licensing and assets

The original repository has no license grant. Its complete history attributes
the code to NukeItGames, with no conflicting third-party source notices found.
The owner explicitly selected **MIT** for this publication on November 13, 2026.
The new LICENSE names NukeItGames and does not claim ownership of Unreal Engine.
This attribution review is not independent proof of all code provenance.

Five historical PNG blobs were inventoried. They are toolbar artwork, not
screenshots or demos. Larger variants contain software/EXIF metadata and have no
recorded redistribution terms. All old artwork is excluded from publication.
The new 64×64 icon is original procedural geometry with its complete standard
library generator included; its PNG has only IHDR, IDAT, and IEND chunks.
The documentation SVG is an original explanatory diagram explicitly labeled as
an illustration, not a capture. No purchased assets, engine assets, or fake
runtime media are published. Engine primitives/materials are referenced from an
installed engine only. See the separate third-party notices.

## Dependencies and CI boundary

The plugin declares **16 engine modules**, all supplied by Unreal Engine. There
is no third-party plugin dependency, bundled engine implementation, vendored
library, package manifest, or lockfile to audit. The original project name in
CreatedBy is replaced with the public owner's name. Engine modules remain intact
because they support the existing functionality. No engine version was installed
for advisory scanning; no claim of zero engine vulnerabilities is made.

Repository scripts use only Python's standard library. GitHub Actions runs
engine-free checks on Linux and Windows, with a separate checksum-verified
Gitleaks full-history scan. GitHub checkout/setup-python actions are pinned to
verified upstream tag commit hashes, permissions are read-only, credentials are
not persisted, and no secret or self-hosted runner is required. CI does not fetch
Unreal Engine or claim an Unreal build.

## Compatibility and publication gate

The source README names UE 5.8.2 as the development target. Version-sensitive
toolbar, widget, and ITF integration remains uncompiled and unverified here.
No Unreal installation, BuildPlugin/UBT toolchain, or editor executable was found
in this environment. The five tests remain test definitions, not passed tests.
Toolbar cycling, actual face dragging, cancellation, and undo/redo still need
the documented workstation walkthrough. No genuine capture or demo was available.

Security/licensing review passes for publishing the reviewed source with these
limits. Only the new portfolio repository is eligible for pushes. Publication
verification compares the remote tree with this snapshot and rechecks the
original repository's private visibility and refs. See VALIDATION.md for results.
