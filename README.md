C-- WANNABE LANGUAGE


СОСТОЯНИЕ НА 12.05.2024:
Синтаксис по большей части идентичен СИшному. 
in- ввод ОДНОЙ переменной.
out- вывод ОДНОЙ переменной.
(я оставил возможность указать количество аргументов после скобок в объявлениях, но это никак не используется)
ret- вовзрат из функции указанного значения/переменной. можно попробовать его перегрузить, но больше одного 
аргумента он не вернет
Все три вызова реализуются как eval: = <название> 
После знака степени нужны скобки.
Сложные условия не поддерживаются.
Функция main обрабатывается не так же, как остальные: бэкенд вместо ret печатает ей hlt(main не принимает и не 
возвращает ничего).
if в проце реализованы через отрицание условий (условно, if(d != 0){...} обработается бэкендом как jz <...>)
фигурные скобки обязательны после названий функций и if/while.
if/while сохраняются фронтендом с типом FUNCNAME; обрабатываются они все равно по-своему, но путаница имеется.
Хотя бэк и фронт нормально работают с float, процессор их не видит и работает с интами(my bad, не успел)
Возвратный аргумент всегда передается в ax.
Передача в функции не более 4 аргументов (по числу регистров; можно передавать через стек, но не успел- снова my bad).
Фронт распознает названия математических функций вне зависимости от регистра(lN, CoS).
Для работы с переменными в main и других функциях у проца разные команды. Адрес обращения к переменным в первом случае
абсолютный, во втором- относительно смещения. 
Сложные вызовы рекурсий с вероятностью 99% положат проц (my bad x3)(хотя через цикл все отработает, см. фибоначчи)

РТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТРТ