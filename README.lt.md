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

## Palaikomi sekimo įrenginiai

| Įrenginys | PlatformIO aplinka |
| --- | --- |
| LilyGO T-Echo Lite su KeyShield | `LilyGo_T-Echo-Lite_keyshield_zygis` |
| ThinkNode M5 | `ThinkNode_M5_zygis` |

T-Echo Lite naudojamas „KeyShield“ garsiakalbis ir klaviatūra. Žalias LED
praneša apie GPS vietos nustatymą, mėlynas – apie išsiųstą vietos ataskaitą.
M5 naudojamas integruotas garsinis signalizatorius; jo fizinis GPS jungiklis
turi likti įjungtas.

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

1. Įsidiekite „PlatformIO“ ir atsisiųskite šią saugyklą.
2. Sukompiliuokite savo įrenginio aplinką:

   ```sh
   pio run -e LilyGo_T-Echo-Lite_keyshield_zygis
   # Arba:
   pio run -e ThinkNode_M5_zygis
   ```

3. Vadovaukitės [įrašymo ir nustatymo instrukcija](README.md#build-and-get-started).
   M5 keičiamas atminties skaidinių išdėstymas: pirmą kartą reikia viso įrašymo
   per USB ir ankstesnių duomenų atsarginės kopijos.
4. Programėlėje sukurkite privatų kanalą su nauju atsitiktiniu raktu. USB terminale
   įjunkite CLI atkūrimo režimą ir pasirinkite kanalo numerį:

   ```text
   status channels
   status channel 1
   status interval 300
   gps on
   verify
   exit
   ```

CLI režimas įjungiamas palaikius naudotojo mygtuką per pirmąsias aštuonias
sekundes po paleidimo; terminalo greitis – 115200. `1` pakeiskite savo kanalo
numeriu. Išsami informacija – [sekimo vadove anglų kalba](docs/techo_lite_status.md).

## Dabartinė būsena

Tai **bendruomenei skirta bandomoji versija**. Automatiniai programinės įrangos
testai ir vietiniai abiejų įrenginių kompiliavimai atlikti, tačiau veikimas su
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
