# Параллельное программирование

Здесь находятся материалы для лабораторных работ по параллелкам в ЛЭТИ.

## Постановка задачи

Все программы решают задачу `n` тел.

Формат входных файлов следующий.

```
G r dt n steps
m x y z vx vy vz	//n раз
```

где `G` -- гравитационная постоянная, `r` -- радиус тел, `n` -- их количество, `dt` -- сколько времени проходит за шаг симуляции, `steps` -- количество этих шагов, `m` -- масса тела, `x, y, z` -- его координаты, `vx, vy, vz` -- его скорость.

По окончание работы программа пишет данные о каждом симулированном теле и время своей работы.

## Компилляция и запуск

### Последовательная программа

Для компилляции последовательной программы используется команда

`$ gcc sequential/n-bodies.c -o sequential/n-bodies.nexe -lm`

Для запуска с тестовыми данными

`$ ./sequential/n-bodies.nexe tasks/debug/1-step/task.txt path/to/solution.txt`

Время работы будет выведено в `stdout`, а результат работы будет схож с тем, что находится по адресу `tasks/debug/1-step/solution.txt`.

### Open MP

Для компилляции

`$ gcc open-mp/n-bodies.c -o open-mp/n-bodies.nexe -lm -fopenmp`

Для запуска требуется сначала указать количество используемых процессов

`$ export OMP_NUM_THREADS=4`

В остальном нет отличий.

### OpenCL

Я использовал эти две ссылки:

- https://forums.linuxmint.com/viewtopic.php?t=362544
- https://github.com/KhronosGroup/OpenCL-Guide/blob/main/chapters/getting_started_linux.md

[Полезная ссылка для понимания терминологии.](https://youtu.be/AJnIdwbP5jI)

Функции kernel пишутся отдельно от main в своем особом формате, и их еще недо прочитать.

https://github.com/HandsOnOpenCL/Exercises-Solutions/blob/master/Solutions/Exercise04/C/vadd_chain.c

## Результаты экспериментов

Понимаю-понимаю, но это лабораторные, отстаньте.

### Последовательная программа

![sequential](res/sequential.png)

### Open MP

![open-mp](res/open-mp.png)