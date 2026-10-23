# ESP32_V6_myref

Dit is mijn referentieproject voor de ESP32-S3 Mini. Ik gebruik het als
verzameling van mijn eigen, herbruikbare ESP-IDF-componenten. Nieuwe componenten
kunnen hier worden toegevoegd en in dit project worden uitgeprobeerd.

## Componenten

| Naam | Bestaat | Map |
|---|---|---|
| `myGPIO` | Ja | `components/basic/` |
| `myESP_S3_mini_board` | Ja | `components/boards/` |


## Als je hier een component van wil gebruiken

Als je van dit project een component wil overnemen, maak dan in de map main een nieuwe file "idf_component.yml" aan 
(als je dat nog niet had)

VOOR HTTPS:
En voeg onderstaande tekst hierin toe (dit is het voorbeeldje voor de gpio files)

```
dependencies:

  myGPIO:
    path: components/basic/myGPIO
    git: https://github.com/KNaelaerts/ESP32_Periferie.git

```


VOOR SSH:
En voeg onderstaande tekst hierin toe (dit is het voorbeeldje voor de gpio files)

```
dependencies:

  myGPIO:
    path: components/basic/myGPIO
    git: git@github.com:KNaelaerts/ESP32_Periferie.git

```

## omgang moet git
### 1ste keer
* ga naar github en maak een nieuwe repo aan (en copy de link)
* "git init"    dit maakt een verborgen folder met alle eigenschappen van git aan
* "git remote add origin +url"  dit legt de link tussen jouw project en de repo die je hebt aangemaakt

### als je veranderingen wil doorvoeren
* "git add ."
* "git commit -m +info"
* "git branch -M main"          -hier kies je de branch die je wil
* "git push -u origin main"

### als je een nieuwe branch wil starten
* "git switch main"                 -ga eerst naar de branch waarvan je vertrekt
* "git pull"                        -zorg dat die up-to-date is
* "git switch -c +naam"             -maakt de nieuwe branch aan en gaat er meteen naartoe
* werk zoals gewoonlijk: "git add ." en "git commit -m +info"
* "git push -u origin +naam"        -publiceert de nieuwe branch op github

### pullen
* "git pull"    haalt de nieuwste wijzigingen van github op en voegt ze meteen samen met je huidige branch
* het is eigenlijk een "git fetch" gevolgd door een "git merge"
* doe dit best voor je aan iets nieuws begint, en commit eerst je eigen werk

### fetchen
* "git fetch"   haalt de nieuwste wijzigingen van github op, maar past je eigen bestanden niet aan
* zo kan je eerst bekijken wat er veranderd is, bv. met "git log HEAD..origin/main" of "git diff origin/main"
* "git branch -a" toont alle branches, ook die van github
* wil je de wijzigingen daarna toepassen, gebruik dan "git merge origin/main" (of gewoon "git pull")

### twee branches samenvoegen (mergen)
* "git switch main"                 -ga naar de branch waarin je wil samenvoegen (de ontvanger)
* "git pull"                        -zorg dat die up-to-date is
* "git merge +naam"                 -voegt de andere branch toe aan de huidige branch
* "git push"                        -publiceert het resultaat
* bij een conflict zegt git welke bestanden problemen geven: open ze, kies in de markeringen (`<<<<<<<`, `=======`, `>>>>>>>`) wat je wil behouden,
  en doe dan "git add ." en "git commit"
* "git merge --abort"               -breekt de merge af als je terug wil naar het begin

## myflow

Ik probeer bij elke component een voorbeeld toe te voegen in de vorm van een flowchart.
Die flowcharts maak ik met de myflow-extensie. Deze extensie is privé, omdat ze nog maar pas gemaakt is.

Heb je interesse of vragen over myflow? Spreek mij gerust aan.
