# Football Manager (Console Simulation)

A deep, text-based football management simulation written in C++. Take control of a club, manage your squad, compete in domestic leagues and continental competitions, and guide national teams – all from the terminal.

> **Status:** Early Development  
> **Language:** C++ (C++11 or later)  
> **Interface:** Console (text menus)

---

## Features

### 🧑‍💼 Player System
- 1–20 attribute scale: technical, physical, mental, hidden (potential, injury proneness, etc.)
- 19 playstyles (e.g., DLP, Inside Forward, Raumdeuter)
- 20 player traits (e.g., “Tries Killer Balls”, “Gets Into Opposition Area”)
- Monthly progression based on age, match performance, training, and mentorship

### ⚽ Team Management
- Senior, youth, and academy squads
- Facilities that influence player development
- Tactical setup (formation, mentality, roles)
- Head-to-head records, club records, and trophy cabinet
- All-time player stats for the club

### 🏟️ Match Engine
- Possession-based grid simulation
- Detailed match events (goals, cards, substitutions)
- Advanced stats: xG, progressive passes, duels, tackles
- Per‑player match statistics

### 🏆 Competitions
- **Domestic Leagues** – Home & away schedules, league table, promotion/relegation
- **Continental Competitions** – UEFA Champions League, Europa League, Conference League
  - Coefficient system, Swiss-style scheduling, knockout rounds
- **International Management** – National teams, World Cup and Euros qualification, tournament brackets

### 💰 Transfers & Scouting
- Dynamic market values based on age, contract, form, and traits
- Scouting knowledge tiers affecting report accuracy
- TransferEngine handles bids, negotiations, and deadline day

### 📊 Data Hub
- Automatically generates opponent reports and squad analyses
- Tracks historical data for deeper insights

### 💾 Save & Load
- All world data stored in a single `save_game.json` file
- Also loads initial world from `world_data.json` and `countries_data.json
- 
