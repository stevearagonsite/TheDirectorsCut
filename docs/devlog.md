# The Director's Cut — Dev log (2026 Epic AI MegaJam)

> A 2.5D metroidvania where you are a film director trapped inside your own unfinished movie. You don't just fight the world, you **direct** it: switch the set lights on and off to make platforms exist, and rewind the take to bring a collapsed bridge back.

Solo entry by **stevearagonsite**, built in Unreal Engine 5.8 with a team of AI agents across two machines. This log is written as we go; the decision record lives in [`docs/product/decisiones.md`](product/decisiones.md).

## How we worked: one human, several agents, two computers

- **Mac (direction):** design, C++, docs and review. It has no Unreal or Blender installed.
- **Windows PC (studio):** Unreal Engine 5.8, Blender 5.3, builds, tests and evidence.
- **Channel:** git is the source of truth. Every task is a file in [`docs/tasks/`](tasks/) on its own branch. VirtualKvm (our own LAN tool) is only the "doorbell" between the Claude Code agents on each machine. The protocol is in [`docs/pipeline/protocolo-mac-windows.md`](pipeline/protocolo-mac-windows.md).
- **Agent roles on the Mac:**
  - *gamedesigner*: design, specs and C++.
  - *coordinator-kvm*: the Windows task queue.
  - *test-e4*: the PC workers and mailboxes.
  - *virtual kvm*: the messaging tool itself.
- **Mid-jam fix:** the messaging daemon hung when one agent stopped reading its socket. The agents diagnosed it, shipped VirtualKvm 0.1.2 → 0.1.4 (per-session mailboxes) and kept going.

## Timeline (Nov 15, ART)

| Time | Milestone |
|---|---|
| 12:48 | Repo bootstrap: UE project, C++ prototype of the **Lights** and **Rewind** mechanics, greybox level script |
| 13:00–14:30 | Design grilling: protagonist, combat, power cost, art tone, the "Cut" power (D-001…D-010) |
| 13:15 | Game-design docs audited with game-docs skills: feature notes for movement, lighting and time |
| 14:00 | Art direction from Steve's own concepts: character, level/camera, concept video (D-011, D-012) |
| 15:35 | Orthographic model sheet of the director generated with Gemini |
| 15:40 | First 3D blockout of the director, built **live by an agent driving Blender** through blender-mcp on the PC |
| 15:50 | Project moves to UE 5.8 (D-013) |
| 16:00 | Jam plan: Tripo for the rigged character, Hyper3D Rodin for set props, packaged build at 19:00, freeze at 21:00 |

## Design in one page

- **Pillar: Direction.** Every feature must answer "does this make the player direct the scene?"
- **Jam scope:** one greybox set, two powers.
  - **Lights:** set pieces exist only in light (yellow) or only in darkness (violet).
  - **Rewind:** objects "on take" (cyan) rewind while you hold the button.
- **Visual language:** the same colours run from greybox to final art, so mechanics stay readable.
- **Post-jam:** three film "departments" of powers (Editing, Set Design, Cinematography), a shared film-reel resource, and one movie genre per zone (noir first).

## Tools used

| Tool | Used for |
|---|---|
| Unreal Engine 5.8 | Game, C++ gameplay, Python editor scripting |
| Claude Code (Anthropic) | All agents: design, C++, docs, task coordination, review |
| Blender 5.3 + blender-mcp | Agent-driven character blockout and props |
| Tripo | Image → rigged, animated 3D character |
| Hyper3D Rodin | Image/text → 3D set props |
| Google Gemini (gemini-3-pro-image) | Orthographic model sheet of the protagonist from the concept art |
| VirtualKvm | Agent-to-agent messaging between the Mac and the PC |
| Git + Git LFS + gitleaks | Source of truth, binaries, secret scanning on a public repo |

## Log

- **16:00:** Locked the jam plan. Wrote `tdc.ps1 package` (one-command Win64 build + zip) and skeletal-mesh/AnimBP support for the director in C++, each on its own branch so the build path never waits for art.
