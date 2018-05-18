# TDIU16 - Pintos

#### ToDo:

* - [x] 170509: Fixa test för create file test.
* - [ ] 180518: Kontrollera varför inte test longrun_nowait fungerar med för många childs. (ca 10 50)
* - [x] 180518: Fixa varför comman_line i arguments i start_process fuckar - Felet var en semafor som låg innan utskriften.
`

#### Changelog:

* 170509: Fått pintos att kompilera
* 180410: Redovisat Deadline 1
* 180424: Refaktoriserat syscall_handler och flyttat repot till GitLab
* 180518: Fixat plist.c så att insert och remove fungerar => exit-staus fungerar.

## Contribute

```sh
git checkout develop
git pull origin master
# do your shit
git commit -am 'shit done'
git checkout master
git merge develop
git push origin master
```
