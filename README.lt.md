# MeshCore Solo Žygis

[English](README.md) · **Lietuviškai**

**GPS navigacija, privatus vietos bendrinimas ir žinutės per LoRa tinklą.**

„Žygis“ – [MeshCore Solo](https://github.com/MarekZegare4/MeshCore-Solo)
pagrindu kuriama programinė įranga, skirta kelionėms, žygiams ir eksperimentams
su Lietuvos „MeshCore“ bendruomene. Įrenginio ekrane galima skaityti ir rašyti
žinutes, išsaugoti kelionės taškus, matyti kryptį bei atstumą ir eksportuoti
GPS maršrutą GPX formatu.

LoRa ryšiui nereikia mobiliojo interneto. Ryšio nuotolis priklauso nuo vietovės,
antenų ir pasiekiamų tinklo mazgų. Ataskaitoje esančiai „Google Maps“ nuorodai
atidaryti telefone gali reikėti interneto.

## Kuo papildytas „Solo“?

Pasirenkamos šeimos žaidimų konfigūracijos turi šešis vietinius žaidimus ir
„Tic-Tac-Toe“ per privatų kanalą. Atskira [„Veylith Arena“ CPU peržiūra](docs/veylith-arena.md)
prideda trijų būtybių komandą, rankinį XP skyrimą savybėms, evoliucijas ir kovas
su kompiuteriu, naudojant prototipinius piešinius. Kiekviena būtybė turi keturias
gebėjimų vietas; platesnis pasirinkimas priklauso nuo lygio, elementų ir nuolat
išugdytų savybių. Valdymas ir įrenginių ribos
aprašyti [žaidimų gide](docs/games.md). „Arena“ PvP dar neįgyvendintas.

- **Privačios vietos ataskaitos:** koordinatės, žemėlapio nuoroda ir apytikris
  baterijos įkrovos procentas siunčiami į pasirinktą privatų kanalą.
- **GPS valdymas:** gavęs naują vietos nustatymą ir parengęs ataskaitą, GPS
  užmiega iki kito intervalo. Radijas toliau klausosi žinučių ir komandų.
- **Aiški vietos būsena:** ataskaita nurodo, ar vieta nustatyta naujai, ar rodoma
  paskutinė žinoma vieta, ar vieta dar nežinoma.
- **Nuotolinės komandos:** galima paprašyti vietos, pakeisti ataskaitų intervalą,
  pristabdyti sekimą arba paleisti garsinį signalą.
- **Pranešimai ekrane:** konkrečiam įrenginiui adresuota žinutė sukelia tris
  trumpus signalus ir lieka ekrane, kol paspaudžiamas fizinis valdiklis.
- **Tiesioginių retransliatorių paieška:** parodomi pasiekiami retransliatoriai
  ir jų signalų duomenys. Komanda `!triangulate` koordinačių neapskaičiuoja.

## Įrenginių palaikymas

Projekto tikslas – navigacijos, žinučių ir privataus vietos bendrinimo funkcijas
pritaikyti „MeshCore Solo“ palaikomai įrangai. Galimybės priklauso nuo įrenginio
GPS, ekrano, valdiklių, garso įrangos ir pasirinktos kompiliavimo konfigūracijos.

Išsaugotos ankstesnės „Solo“ konfigūracijos. „Žygio“ funkcijos dar pritaikomos
skirtingiems įrenginiams, todėl esanti plokštės konfigūracija savaime nereiškia,
kad visos sekimo funkcijos joje įjungtos ar patikrintos. Žr.
[įrangos aprašą](README-Solo.md#supported-devices), [kompiliavimo aplinkas](solo/)
ir [dabartinės integracijos informaciją](docs/tracking.md).

## Komandų pavyzdžiai

Pavyzdžiuose įrenginys pavadintas `Tracker01`. Naudokite tikslų savo įrenginio vardą.
Komandos siunčiamos į sukonfigūruotą privatų kanalą:

```text
@[Tracker01] !kur
@[Tracker01] !interval 300
@[Tracker01] !power down
@[Tracker01] !power up
@[Tracker01] !tikra Grįžkite į stovyklą
@[Tracker01] Patikrinkite radiją
```

Be `@[Tracker01]` prefikso komanda taikoma visiems ją gavusiems to kanalo
sekimo įrenginiams. `!help` pateikia komandų sąrašą. `!interval 0` išjungia
periodines užklausas; `!power down` sustabdo ir vykstantį GPS vietos nustatymą.

## Kaip pradėti?

1. Pasirinkite savo įrenginio konfigūraciją ir patikrinkite, kurios funkcijos joje įjungtos.
2. Įsidiekite „PlatformIO“, atsisiųskite saugyklą ir sukompiliuokite pasirinktą aplinką:

   ```sh
   git clone https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis.git
   cd Meshcore-Solo-Zheegis
   pio project config
   ```

   Kompiliuokite komanda `pio run -e YOUR_ENVIRONMENT`, vietoj `YOUR_ENVIRONMENT`
   įrašydami savo konfigūracijos pavadinimą.
3. Įrašykite programinę įrangą pagal [savo plokštės instrukciją](README-Solo.md#flashing).
   Prieš įrašydami patikrinkite konkrečios konfigūracijos atminties keitimo reikalavimus.
4. Suderinkite radijo nustatymus su savo grupe ir suderinamoje programėlėje
   sukurkite arba importuokite privatų kanalą.
5. Pagal [pasirinktos aplinkos sekimo vadovą](docs/tracking.md)
   nustatykite sekimo kanalą bei ataskaitų intervalą ir išbandykite žinutes bei komandas.

Konfigūravimo būdas priklauso nuo pasirinktos aplinkos. Veikimas su fiziniais
įrenginiais dar turi būti patikrintas.

[Šeimos sekiklio profilis](docs/family-profile.md) išlaiko GPS maršrutus, sekimą
ir žaidimus, bet atsisako bendros kontaktų knygos ir pokalbių istorijos. L1 Pro
aplinka: `WioTrackerL1_zygis_family`; pradžios ekrane išmatuota apie 141 KiB
laisvos RAM.

## Dabartinė būsena

Pasirenkama [žaidimų bandomoji versija](docs/games.md) L1 OLED aplinkoje prideda
šešis vieno žaidėjo žaidimus: „Kryžiukus ir nuliukus“, „Snake“, šaškes,
„Battleship“, „Scorched Earth“ ir „Texas Hold’em“. Taip pat yra tinkliniai
„Kryžiukai ir nuliukai“, spartusis paleidimas ir atminties atlaisvinimas uždarius
ekraną. Žaidimą tarp dviejų fizinių
radijo įrenginių dar reikia patikrinti. `WioTrackerL1_zygis_games_production`
aplinka skirta versijai be USB derinimo priemonių.

Tai **bendruomenei skirta bandomoji versija**. Automatiniai programinės įrangos
testai ir vietiniai programinės įrangos kompiliavimai atlikti, tačiau veikimas su
fiziniais įrenginiais dar turi būti patikrintas. GPS be vietos nustatymo laiko
apribojimo gali ilgai likti įjungtas. Navigacija naudoja sekimo metu gaunamus
GPS duomenis; tarp ciklų GPS miega.

Automatinės viešos vietos reklamos ir „Solo“ automatinis vietos bendrinimas
sekimo aplinkose išjungti. Kiekvienas privataus kanalo rakto turėtojas gali siųsti
valdymo komandas. Ataskaitos įtraukimas į siuntimo eilę negarantuoja pristatymo.

## Prisidėkite

[Užduokite klausimą arba pasidalykite patirtimi](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/discussions),
[praneškite apie klaidą](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/issues/new/choose)
arba pasiūlykite patobulinimą. Lietuviški ir angliški pranešimai laukiami.

Žr. [projekto planą](ROADMAP.md), [pakeitimų sąrašą](CHANGELOG.md) ir
[prisidėjimo gaires](CONTRIBUTING.md). Projektas remiasi „MeshCore“ bei „MeshCore
Solo“ autorių darbu; licencijų informacija pateikta [angliškame README](README.md#credits-and-licensing).
