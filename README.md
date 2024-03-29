# TranslatorProject
Это интерпретатор собственного языка "Кастрат", названного так из-за того, что язык повторяет синтакс C++, но не содержит множество его инструментов.

Все возможности языка подробно описаны в [Руководстве пользователя](https://drive.google.com/file/d/1ktQxen7Bq-s18VKdA5Zn4YiSYmB63Mh8/view?usp=sharing), а особенности реализации в [Документации разработчика](https://drive.google.com/file/d/1IhgbXlJS_3TPZRIGhel6qTQlfqW86vX8/view?usp=sharing), однако здесь можно дать основную информацию о проекте.
## Особенности транслятора
Код для исполнения берётся из файла input.txt, расположенного в том же каталоге, что и исполняемый файл транслятора (или в корне проекта Visual Studio). Транслятор начинает сборку сразу после запуска. Открывается консоль. В ней отображаются ошибки сборки и, если их нет, консоль выполняет роль терминала для работы собранной программы. Там же выводится сообщение о завершении её работы. 

В проекте находится 5 TXT файлов:
- input.txt - содержит код вашей программы.
- output.txt - содержит дополнительную информацию о пройденных этапах сборки, не участвует в самой сборке.
- regexes.txt - служебный файл с регулярными выражениями дя разбиения исходного кода на лексемы. 
- serviceWords.txt - служебный файл с зарезервированными словами на английском и русском языках. 
- config.txt - содержит относительные пути всех остальных txt файлов.

Фактически, все типы данных в языке определяются файлами regexes.txt и serviceWords.txt.
Дополнительная информация о сборке собирается в файле output.txt.

## Особенности языка
Язык копирует синтаксис C++, однако не имеет функции main и каких-либо подключаемых файлов. Весь код пишется в input.txt и исполняется от начала файла. Пример кода:
```
int a = 1;
a += 2;
out a;
```
Обо всех частях языка можно подробно узнать в [Руководстве пользователя](https://drive.google.com/file/d/1ktQxen7Bq-s18VKdA5Zn4YiSYmB63Mh8/view?usp=sharing).
Язык поддерживает типы `int`, `bool`, `float`, `char`, `string`, `void`(для функций). Также имеются функции и шаблоны. Пример функций, связанных косвенной рекурсией.
```
int bar(int);
int foo(int f) {
  return bar(f);
};
int bar(int b) {
  return foo(b);
};
foo(2);
```
Если функция должна возвращать зачение, но не достигла return по завершении, то выводит стандартное(пустое) значение типа. Для int - 0, для string - "" и т.д. 

Также имеются многомерные массивы. Пример использования.
```
int n = int(in);
char[6] str = "строка"; // символ конца не учитывается
int[n][2] arr;
for (int i = 0; i < n; ++i) {
  int[2] row; // нет инициализации в скобках
  row[0]=int(in);
  row[1]=int(in);
  arr[i] = row;
}

for (int i = 0; i < n; ++i) {
  out arr[i][0] + " ";
  out arr[i][1] + "\n";
}
```
## Странности языка
Стоит сказать, что массивы запрещено передавать в функции. Также есть возможность создавать вложенные друг в друга функции. Коллизия имён для функций запрещена. Комментарии можно вставлять в абсолютно любую часть кода. Они уничтожаются ещё до начала лексического анализа.
## Русский и английский
Язык имеет поддержку кода на русском языке. Если на английском представлены обычные термины и ошибки, то на русском используются весёлые термины, а ошибки пытаются шутить. Каждое зарезервированное слово и ошибка имеет свой русский аналог. 

Имена переменных и функций подчиняются ограничениям на имена C++, однако допускается использование символов русского алфавита. В связи с этим в одном пространстве имён могут находиться имена визуально идентичные. Например, `а` и `a` - разные переменные, потому что одно написано на русском.
Вот пример рабочего алгоритма для нахождения функции Эйлера, однако написанного с учётом этой особенности "на собаках".
```
кек собака(кек cобака)
{ 
  кек сoбака = cобака;
  покуда(кек собaка = 2; собaка * собaка <= cобака; ++собaка)
  {
    кабы(cобака % собaка == 0) 
    {
      доколе(cобака % собaка == 0)
        cобака /= собaка;
      сoбака -= сoбака / собaка;
    }
  }
  кабы(cобака > 1)
    сoбака -= сoбака / cобака;
  return сoбака;
};
```
Очевидно, в одно пространстве имён может быть 16 разных переменных "собака".
## Особенности проекта
Транслятор - командный проект. Он сделан двумя учащимися 11 класса: [stepan2409](https://github.com/stepan2409) и [KIrillPal](https://github.com/KIrillPal). Первый заложил фундамент проекта - настроил подкючение, чтение и запись всех файлов. Написал лексический анализатор, занимался документацией и нахождением багов. Также приложил руку к исполнению и всем остальным частям проекта.
Второй написал грамматику языка, семантический, синтаксический анализаторы. Сделал исполнение программы, обработу всех ошибок, исправил тысячи багов и добавил русский язык. Это учебный проект, для его создания использовались конспеты ВМК. Получилось гораздо больше, чем от нас требовали, и это один из самых затратных для KIrillPal(и, возможно, для stepan2409) проектов по времени и силам.
