# OpenCatESP32 — OpenCat Framework on ESP32/BiBoard

🚀 **[Quaddle](https://prelaunch.com/projects/petoi-quaddle-your-perfect-tinkering-companion?utm_source=github&utm_medium=code&utm_campaign=github-opencat&utm_content=esp32-readme-hero-banner) — Petoi's newest OpenCat-lineage quadruped — launches on Kickstarter August 2026.** [Reserve your spot →](https://prelaunch.com/projects/petoi-quaddle-your-perfect-tinkering-companion?utm_source=github&utm_medium=code&utm_campaign=github-opencat&utm_content=esp32-readme-hero-banner) (details below)

OpenCatESP32 runs the OpenCat quadruped robotics framework on [BiBoard](https://www.petoi.com/products/biboard-esp32-development-board-for-quadruped-robot?utm_source=github&utm_medium=code&utm_campaign=github-opencat) — an ESP32-based development board designed for multi-degree-of-freedom legged robots with up to 12 servos. Developed by [Petoi](https://www.petoi.com?utm_source=github&utm_medium=code&utm_campaign=github-opencat), the maker of futuristic programmable robotic pets.

This is the codebase for current-generation Petoi hardware. If you're on the older NyBoard (ATmega328P), see the main [OpenCat repo](https://github.com/PetoiCamp/OpenCat).

[![BittleESP32](https://github.com/PetoiCamp/NonCodeFiles/blob/master/gif/BiBoard.gif?raw=true)](https://www.youtube.com/watch?v=GTgps_H990w)

[![BittleGap](https://github.com/PetoiCamp/NonCodeFiles/blob/master/gif/gap.gif?raw=true)](https://youtu.be/1qhNRSQTcG4)

*Click either GIF to watch the demo.*

---

## Coming Soon: Quaddle mini robot dog
![](https://github.com/PetoiCamp/NonCodeFiles/blob/master/gif/quaddleCover.gif?raw=true)

[Quaddle](https://prelaunch.com/projects/petoi-quaddle-your-perfect-tinkering-companion?utm_source=github&utm_medium=code&utm_campaign=github-opencat&utm_content=esp32-readme-quaddle-section) is Petoi's newest quadruped, launching on Kickstarter **August 2026** — a mini desk robot built on the same OpenCat lineage as Bittle and Nybble. What makes it worth a look: it's a full quadruped running on just 4 servos instead of the usual 8–12, which forces genuinely different gait-design and leg-coordination solutions to still get all four legs walking, running, and balancing. Its servos are also position-feedback — readable, not just drivable — which is what makes Puppet Mode possible: hand-guide the legs and record a motion directly, no code required to capture a new behavior. We'll also share 3D-printable shells and mounts for customization. Same open platform this repo already supports for gait research, RL, and sim2real work — a hands-on physical AI platform, not a toy version of the idea.

**Source code is not yet public.** It's an upgraded version of the OpenCat project, and its ESP32-S3 code structure is closer to this repo than to the main NyBoard-based [OpenCat repo](https://github.com/PetoiCamp/OpenCat-Quadruped-Robot). We plan to open source it before Quaddle delivery. Watch this repo and [r/petoi](https://www.reddit.com/r/Petoi/) for the announcement, or [reserve a spot](https://prelaunch.com/projects/petoi-quaddle-your-perfect-tinkering-companion?utm_source=github&utm_medium=code&utm_campaign=github-opencat&utm_content=esp32-readme-reserve-spot-link) to get notified at launch.

---

## About the Project

Inspired by Boston Dynamics' Spot, [Dr. Rongzhong Li](https://www.linkedin.com/in/rongzhongli/) started OpenCat in his dorm at Wake Forest University in 2016. The goal was straightforward: make agile quadruped robots affordable and hackable enough for researchers, educators, and makers — not just well-funded labs.

Since then:
- **30,000+ robots shipped**, cumulative across all channels
- **60+ countries**
- **$1M+ raised** across two prior Kickstarter and Indiegogo campaigns
- **20+ academic papers** cite the platform — see [Research Spotlight](https://www.petoi.com/pages/robotics-research-and-academic-applications?utm_source=github&utm_medium=code&utm_campaign=github-opencat)
- One of the most-starred open-source quadruped robot projects on GitHub

---

## Hardware

BiBoard is the control board for:

- 🐶 [Bittle X — mini robot dog & AI robotics kit with voice control](https://www.petoi.com/products/petoi-robot-dog-bittle-x-voice-controlled?utm_source=github&utm_medium=code&utm_campaign=github-opencat) · [buy on Amazon](https://www.amazon.com/dp/B0FNT6TSVT?utm_source=github&utm_medium=code&utm_campaign=github-opencat&utm_content=esp32-readme-amazon-buy-link)
- 🐱 [Nybble Q — mini robot cat & AI robotics kit](https://www.petoi.com/products/petoi-nybble-q-robot-cat?utm_source=github&utm_medium=code&utm_campaign=github-opencat)

The older [Bittle](https://www.petoi.com/collections/robots/products/petoi-bittle-robot-dog?utm_source=github&utm_medium=code&utm_campaign=github-opencat) and [Nybble](https://www.petoi.com/collections/robots/products/petoi-nybble-robot-cat?utm_source=github&utm_medium=code&utm_campaign=github-opencat) (NyBoard) are discontinued. BiBoard is the current platform.

---

## Why BiBoard Over NyBoard?

The ATmega328P (NyBoard) gets the job done for locomotion. BiBoard is for when your **robotics programming** needs more headroom:

- **ESP32 dual-core @ 240 MHz** — handle real-time servo coordination and a perception pipeline simultaneously
- **Wi-Fi + Bluetooth built in** — wireless control and data streaming without a dongle
- **Up to 12 servos** — full 12-DOF configurations
- **Arduino IDE compatible** — same workflow, more horsepower
- **Open source** — hardware and software both forkable

If you're building an open source robot dog for research, running ROS, deploying a vision model, or just want room to experiment — BiBoard is the right foundation.

---

## Board Configuration

**Arduino IDE settings (ESP32 Dev Module):**

| Setting | Value |
|---|---|
| Upload Speed | 921600 |
| CPU Frequency | 240 MHz (WiFi/BT) |
| Flash Frequency | 80 MHz |
| Flash Mode | QIO |
| Flash Size | 4 MB |
| Partition Scheme | Default 4MB with spiffs |
| Core Debug Level | None |
| PSRAM | Disabled |

Full setup: [Upload Sketch for BiBoard](https://guide.petoi.com/arduino-ide/upload-sketch-for-biboard)

---

## What People Have Built

- [AI and computer vision applications](https://www.petoi.com/blogs/blog/tagged/showcase+ai?utm_source=github&utm_medium=code&utm_campaign=github-opencat)
- [Raspberry Pi robotics projects](https://www.petoi.com/blogs/blog/tagged/raspberry-pi?utm_source=github&utm_medium=code&utm_campaign=github-opencat)
- [NVIDIA Isaac simulations and reinforcement learning](https://www.youtube.com/playlist?list=PLHMFXft_rV6MWNGyofDzRhpatxZuUZMdg)
- [SLAM with ROS using Bittle and Raspberry Pi](https://www.youtube.com/watch?v=uXpQUIF_Jyk&list=PLHMFXft_rV6MWNGyofDzRhpatxZuUZMdg&index=6)

Academic and research use: [Research Spotlight](https://www.petoi.com/pages/robotics-research-and-academic-applications?utm_source=github&utm_medium=code&utm_campaign=github-opencat)

---

## Education

OpenCat shows up in **AI robotics education** across K-12 programs, community colleges, university labs, and maker spaces — hands-on **physical AI** teaching that goes beyond screen-based coding exercises:

- [Robotics education showcases](https://www.petoi.com/blogs/blog/tagged/showcase+education?utm_source=github&utm_medium=code&utm_campaign=github-opencat)
- [STEM & robotics curriculum resources](https://www.petoi.com/pages/resources-curriculum-stem-coding-robot?utm_source=github&utm_medium=code&utm_campaign=github-opencat)
- [Robotics competitions](https://www.petoi.com/blogs/blog/robot-competitions-with-petoi?utm_source=github&utm_medium=code&utm_campaign=github-opencat)

---

## Community & Discussion

- [r/OpenCat](https://www.reddit.com/r/OpenCat/) — firmware code, framework hacking, extending and porting OpenCat
- [r/Petoi](https://www.reddit.com/r/Petoi/) — hardware Q&A, builds, quadruped coding, curriculum, 3D-printed parts, general discussion

---

## Resources

- [Petoi Guides](https://guide.petoi.com/)
- [AI reinforcement learning with Petoi](https://www.petoi.com/blogs/blog/petoi-robot-ai-reinforcing-learning?utm_source=github&utm_medium=code&utm_campaign=github-opencat)
- [FinoBot — AI robot dog with Raspberry Pi and ROS2](https://www.petoi.com/blogs/blog/finobot-ai-robot-dog-bittle-x-follows-someone?utm_source=github&utm_medium=code&utm_campaign=github-opencat)
- [Robotics research with Bittle adapting to different surfaces at Haverford College](https://www.petoi.com/blogs/blog/robotics-research-with-bittle-robot-dog-at-haverford-college?utm_source=github&utm_medium=code&utm_campaign=github-opencat)
- [Advanced tutorials by the community](https://www.youtube.com/playlist?list=PLHMFXft_rV6MWNGyofDzRhpatxZuUZMdg)
- [All kits and accessories](https://www.petoi.com/store?utm_source=github&utm_medium=code&utm_campaign=github-opencat)
- [FAQ](https://www.petoi.com/pages/faq?utm_source=github&utm_medium=code&utm_campaign=github-opencat)

Follow the project: [YouTube](https://www.youtube.com/@petoicamp?utm_source=github&utm_medium=code&utm_campaign=github-opencat&utm_content=esp32-readme-youtube-follow) · [Twitter](https://twitter.com/petoicamp?utm_source=github&utm_medium=code&utm_campaign=github-opencat&utm_content=esp32-readme-twitter-follow) · [Instagram](https://www.instagram.com/petoicamp/?utm_source=github&utm_medium=code&utm_campaign=github-opencat&utm_content=esp32-readme-instagram-follow) · [Facebook](https://www.facebook.com/PetoiCamp/?utm_source=github&utm_medium=code&utm_campaign=github-opencat&utm_content=esp32-readme-facebook-follow) · [LinkedIn](https://www.linkedin.com/company/petoi/?utm_source=github&utm_medium=code&utm_campaign=github-opencat&utm_content=esp32-readme-linkedin-follow)
