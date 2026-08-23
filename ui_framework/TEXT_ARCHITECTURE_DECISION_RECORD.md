# UI Framework — Text Architecture Decision Record

## Status

Рабочий архитектурный документ для ветки `fix/sharp-logical-text`.

Этот документ фиксирует:

- исходную архитектурную модель framework;
- место текста в этой модели;
- проблемы, которые появились при эволюции `TextPrimitive`;
- варианты, которые рассматривались;
- почему каждый вариант оказался неудовлетворительным или пока не доказан;
- что подтверждено исходниками и `.md` документацией;
- какие решения считаются отброшенными;
- какие вопросы остаются открытыми.

Документ намеренно не является только описанием текущего кода. Он также фиксирует отрицательные решения, чтобы одинаковые архитектурные идеи не приходилось заново обсуждать.

---

# 1. Основная архитектурная идея framework

Framework не является универсальным UI toolkit и не является CSS/React-подобной системой произвольных пользовательских свойств.

Основная модель ближе к следующей:

```text
Client
  |
  | declares framework-recognized state / semantics
  v
Node / PanelNode / framework-defined component
  |
  | framework interprets that state
  v
Framework subsystem
  |
  +--> LayoutSystem
  +--> InputSystem / hit-test
  +--> ModalSystem
  +--> ScrollSystem
  +--> rendering/runtime
```

Клиент задаёт свойства и semantic state.

Framework сам решает:

- когда делать layout;
- как выполнять measurement/arrangement;
- как делать hit-test;
- как выполнять input routing;
- как инвалидировать layout;
- как выполнять render traversal;
- как интерпретировать framework-recognized semantics.

Клиент не должен вручную вызывать layout engine и не должен синхронизировать внутреннюю инфраструктуру самостоятельно.

Это подтверждается архитектурной документацией: `LayoutSystem` является framework-owned authority для measurement/arrangement; `Node` не выставляет клиенту legacy `measure()/arrange()` lifecycle. Generic свойства `Node` интерпретируются framework subsystems, а concrete components хранят собственный domain state.

---

# 2. Главный принцип boundary

Критическое различие:

```text
PUBLIC CONTRACT
    !=
INTERNAL IMPLEMENTATION
```

Клиент должен знать framework-recognized properties/semantics, если от него требуется реализовать соответствующее поведение.

Клиент не должен знать внутренние реализации подсистем.

Примеры:

```text
Client knows:
    visible
    enabled
    padding
    border
    size
    overflow
    framework-defined node/component contracts

Client does NOT need to know:
    layout queue
    measureRecursive()
    arrangeRecursive()
    hit-test traversal internals
    LinearLayout internals
    TextPrimitive internals
```

Отсюда важное правило для текста:

```text
Client may need to know TEXT SEMANTICS.
Client must not need to know TEXT IMPLEMENTATION.
```

---

# 3. Как framework уже реализует эту модель

## 3.1 Node

`Node` содержит generic framework-recognized properties:

- visibility;
- enabled state;
- focusability;
- capturability;
- position;
- position mode;
- requested size;
- min/max size;
- padding;
- border;
- overflow;
- desired size;
- actual geometry.

`LayoutSystem`, `InputSystem`, `NodeTree` и другие внутренние подсистемы используют эти значения.

Компонент не знает, как именно layout system применяет `padding`, `size`, `overflow` и т. п.

---

## 3.2 PanelNode

`PanelNode` — специализированный framework type.

Он сообщает framework структурную semantic:

```text
Node
  + child ownership
  + child traversal
  + structural composition
```

Клиент получает этот contract через `PanelNode`, а не через знания о внутренностях `NodeTree`.

---

## 3.3 StackPanelNode

`StackPanelNode` добавляет специализированные layout semantics:

- orientation;
- gap;
- main alignment;
- cross alignment.

`LayoutSystem` распознаёт `StackPanelNode` по типу и передаёт его во внутренний linear layout algorithm.

Схематически:

```text
StackPanelNode
    |
    | public framework-defined semantic
    v
LayoutSystem
    |
    v
LinearLayout
```

Клиент знает `StackPanelNode` и его свойства.

Клиент не знает `LinearLayout` и не запускает его самостоятельно.

Это является существующим подтверждённым паттерном framework.

---

## 3.4 Hit-test / Input

`NodeTree` инициирует hit-testing и traversal.

`InputSystem` использует framework-recognized state:

- visible;
- enabled;
- focusable;
- capturable;
- overflow;
- modal boundaries.

Клиент задаёт свойства.

Framework сам применяет их в алгоритмах.

Это второй сильный пример той же модели:

```text
Declarative state
    -> framework interpretation
    -> runtime behavior
```

---

# 4. Историческая модель текста

Исторический repository `Valeri-afk/ui-framework` дал важную информацию.

На раннем этапе `TextPrimitive` создавался как:

```text
internal reusable text primitive
```

и прямо был обозначен как:

- не `NodeTree` object;
- не client-facing service.

В следующем историческом этапе `TextNode` хранил `TextPrimitive` как private member.

Старая модель была приблизительно:

```text
TextNode
  |
  +-- text
  +-- font
  +-- alignment
  +-- color
  |
  `-- private TextPrimitive
```

А text-bearing framework components могли аналогично использовать primitive внутри своей реализации.

Главная историческая граница:

```text
component
    -> private TextPrimitive

client
    -> text/component API
```

а не:

```text
client
    -> TextPrimitive
```

---

# 5. Почему `TextPrimitive` существует

`TextPrimitive` не является просто helper-функцией `drawText()`.

Его необходимость появилась из-за особенностей текста:

- intrinsic measurement;
- text layout;
- SDL_ttf interaction;
- renderer-associated state;
- rasterization;
- caching / reusable state;
- logical/raster font distinction;
- необходимость иметь sharp text при logical scaling.

В текущей ветке особенно важен последний пункт.

---

# 6. Причина новой реализации `TextPrimitive`

В `fix/sharp-logical-text` появилась отдельная проблема:

```text
logical font size = 8 px
        |
        v
logical scaling
        |
        v
обычная растеризация
        |
        v
blurred text
```

Нужна возможность отделить:

```text
logical text/font size
```

от:

```text
rasterized font size
```

Поэтому текущая реализация `TextPrimitive` создаёт отдельную raster font copy и умножает её размер на integer presentation scale.

Это состояние является одной из причин, почему `TextPrimitive` теперь действительно имеет внутреннее mutable state и почему вопрос ownership нельзя сводить только к «маленький stateless helper».

---

# 7. Что изменилось в текущей ветке

В текущей ветке semantic text state был вынесен из `TextPrimitive`.

Теперь `TextPrimitive` получает через аргументы:

- text;
- font;
- horizontal alignment;
- vertical alignment;
- color;
- position;
- size.

А внутри остаются implementation resources/cache, например:

```text
cachedRenderer_
cachedTextFont_
cachedText_
textEngine_
textObject_
rasterFont_
rasterSourceFont_
rasterScale_
rasterFontGeneration_
```

Это изменение является правильным направлением:

```text
Component
    owns semantic state

TextPrimitive
    owns implementation/cache state
```

---

# 8. Первая большая проблема: `TextPrimitive` утёк в `Node`

В текущей ветке возник bridge:

```text
Node
  -> owner_
      -> NodeTree
          -> TextPrimitive
```

через `Node::textPrimitive()`.

То есть базовый `Node` знает конкретную внутреннюю реализацию текста.

Это проблема, потому что `TextPrimitive` не является framework semantic.

Он является implementation detail.

В результате:

```text
Node
    knows TextPrimitive
```

в то время как для layout/input/modal аналогичная утечка отсутствует.

---

# 9. Вторая проблема: framework components начали зависеть от implementation detail

Текущие text-bearing components используют `TextPrimitive` напрямую:

```text
TextNode
Button
MenuItem
TabItem
    |
    v
TextPrimitive
```

Это само по себе допустимо **внутри framework implementation**.

Проблема возникает на уровне client extension.

Если клиент создаёт:

```cpp
class MyComponent : public ui::Node
{
    ...
};
```

и хочет текст, то в текущей модели ему приходится узнать, что существует `TextPrimitive`, и использовать его напрямую или каким-то образом достать его через `Node`.

Это превращает implementation detail в неявный extension contract.

То есть возникает плохой промежуточный вариант:

```text
TextPrimitive supposedly internal
        +
custom component must know it
```

Этот вариант должен быть исключён.

---

# 10. Третья проблема: «просто сделать TextPrimitive public» тоже не подходит

Теоретически можно честно сказать:

> TextPrimitive — публичный contract. Клиентские custom components используют его.

Тогда нет скрытого leakage: всё формально честно.

Но это меняет API framework в нежелательную сторону:

```text
Client custom component
    |
    +-- text properties
    +-- TextPrimitive
    +-- measure
    +-- draw
    +-- rasterization awareness
```

Клиент начинает знать детали text backend.

Это противоречит исходной идее:

```text
client describes semantic state
framework decides how to process it
```

Поэтому этот вариант не выбран.

---

# 11. Четвёртая проблема: один `TextPrimitive` на NodeTree не решает проблему cache так, как кажется

Текущий `TextPrimitive` на самом деле не является multi-entry text cache.

Он хранит примерно:

```text
1 current renderer
1 current logical font
1 current text
1 TTF_Text
1 raster font copy
1 raster source font
```

При смене renderer/font/text предыдущий `TTF_Text` может быть освобождён.

При смене font/scale предыдущий raster font может быть освобождён и создан заново.

Следовательно:

```text
Button A
    -> text A

Button B
    -> text B
```

при одном общем primitive скорее дают:

```text
A
 -> create/cache
B
 -> release A state
 -> create B state
```

чем настоящий cache нескольких текстов.

Поэтому:

```text
shared TextPrimitive
```

не следует автоматически считать необходимым ради cache reuse между всеми компонентами.

Он уменьшает число одновременно живущих SDL_ttf resources, но увеличивает churn между разными текстами.

---

# 12. Пятая проблема: `NodeTree` ownership был введён позже исторической архитектуры

Исторически `TextPrimitive` не был `NodeTree` object.

Он был private implementation object `TextNode`/text-bearing component.

В текущей ветке ownership был перенесён в `NodeTree`, но integration model не была соответственно перестроена.

Получилось:

```text
NodeTree owns TextPrimitive
        +
component needs TextPrimitive
        +
Node exposes TextPrimitive bridge
```

Именно из этого возникла архитектурная утечка.

Следовательно:

```text
NodeTree owns TextPrimitive
```

не является исторически обязательной концепцией.

---

# 13. Решение, которое не подошло №1: `Button : TextNode`

Вариант:

```cpp
class Button : public TextNode
{
};
```

Плюсы:

- Button автоматически становится Node;
- Button получает text API;
- framework может распознавать `TextNode`.

Но проблема фундаментальная.

`TextNode` — конкретный text component, а не универсальная capability.

Он имеет собственный concrete semantic contract:

```text
text
font
horizontal alignment
vertical alignment
color
```

Button может захотеть:

```text
text
font
state-dependent color
fixed alignment
special wrapping policy
press-scale-dependent geometry
```

Наследование от `TextNode` заставляет Button принять весь contract `TextNode`.

---

# 14. Решение, которое не подошло №2: семейство базовых TextNode

Можно попробовать:

```text
TextNode
CenteredTextNode
SingleLineTextNode
WrappedTextNode
EllipsisTextNode
RichTextNode
...
```

А затем:

```text
Button : CenteredTextNode
MenuItem : SingleLineTextNode
...
```

Проблема — комбинационный рост.

Компоненту может понадобиться:

```text
single line
+
custom alignment
+
state-dependent color
+
custom wrapping
```

и существующего base class может не существовать.

Дальше inheritance hierarchy начинает кодировать комбинации text semantics.

Это плохая модель.

Вывод:

> text semantic variations не следует кодировать семейством наследуемых `TextNode` base classes.

---

# 15. Решение, которое не подошло №3: multiple inheritance `Node + TextContent`

Вариант:

```cpp
class Button : public Node, public TextContent
{
};
```

Плюсы:

- Button остаётся Node;
- TextContent выражает text capability;
- нет необходимости наследоваться от concrete `TextNode`.

Минусы:

- framework получает mixin-style hierarchy;
- semantics начинают выражаться через множество базовых классов;
- при развитии framework легко получить:

```text
Node
 + TextContent
 + ScrollContent
 + Selectable
 + Focusable
 + ...
```

- возникает вопрос virtual inheritance и взаимодействия state между capability bases;
- для текущего небольшого framework это слишком тяжёлый механизм.

Решение не выбрано.

---

# 16. Решение, которое не подошло №4: универсальный `ContentNode`

Вариант:

```cpp
class ContentNode : public Node
{
};
```

а затем:

```text
Button : ContentNode
MenuItem : ContentNode
TextNode : ContentNode
...
```

Документация framework прямо предупреждает не создавать generic bases вроде `ContentNode`, `SelectableNode`, `ButtonBase` до появления действительно доказанного общего semantic contract.

Проблема:

- это не concrete semantic;
- он начинает собирать свойства разных domains;
- появляется абстракция ради симметрии.

Решение отклонено.

---

# 17. Решение, которое не подошло №5: `TextProperties` как обязательное поле каждого Node

Вариант:

```cpp
class Node
{
    TextProperties text_;
};
```

Плюсы:

- любой Node автоматически может иметь text;
- framework всегда знает, где искать text;
- custom components не знают `TextPrimitive`.

Проблема:

Разные компоненты не обязаны иметь одинаковый набор text semantics.

Например:

```text
TextNode:
    text
    font
    horizontal alignment
    vertical alignment
    color

Button:
    text
    font
    text color
    fixed center alignment

MenuItem:
    text
    font
    text color
    fixed start/center presentation
```

Общий `TextProperties` либо:

1. становится слишком узким и не покрывает все компоненты;
2. становится большим style bag;
3. начинает смешивать framework text semantics и component-specific presentation policy;
4. превращается в универсальную property system.

`COMPONENT_DESIGN_GUIDE.md` прямо говорит, что component-specific properties обычно должны оставаться properties конкретного компонента, а shared structure должна появляться только при наличии concrete/stable contract.

Следовательно, `TextProperties` как **storage type обязательный для всех компонентов** — не выбран.

---

# 18. Важное уточнение: `TextProperties` как descriptor и `TextProperties` как storage — разные вещи

Это различие критично.

Необязательно заставлять компонент хранить:

```cpp
TextProperties text_;
```

Компонент может хранить:

```cpp
std::string caption_;
TTF_Font *font_;
Color textColor_;
```

а framework-facing semantic contract может создавать временное описание:

```text
component state
    -> framework text descriptor
```

То есть общий type может быть:

```text
INTERNAL / CONTRACT DESCRIPTION
```

а не:

```text
MANDATORY COMPONENT STORAGE MODEL
```

Это сохраняет возможность разных component-specific property sets.

---

# 19. Решение, которое не подошло №6: `Node::drawText()` / `Node::measureText()`

Вариант:

```cpp
class Node
{
protected:
    virtual LayoutSize measureText(...);
    virtual void drawText(...);
};
```

Проблема:

`Node` превращается в text-aware universal base.

Это нарушает separation:

```text
Node
    = generic runtime object
```

а не:

```text
Node
    = universal implementation of every framework subsystem
```

Также custom component получает скрытую обязанность знать text-specific contract.

Вариант не выбран.

---

# 20. Решение, которое не подошло №7: `Node::textPrimitive()`

Текущий исторически возникший вариант:

```cpp
TextPrimitive &Node::textPrimitive()
{
    return owner_->textPrimitive_;
}
```

Это решение является плохим bridge.

Причины:

- `Node` знает internal implementation;
- custom component author может начать использовать `TextPrimitive`;
- internal object становится de facto extension contract;
- конкретный ownership `NodeTree` протекает в base Node;
- для text делается исключение относительно остальных framework subsystems.

`Node::textPrimitive()` должен быть удалён.

---

# 21. Решение, которое не подошло №8: `TextPrimitive` как public extension contract

Формально честный вариант:

```text
TextPrimitive is public.
Custom nodes use it.
```

Тогда custom text component получает:

```text
measure()
draw()
font/rasterization semantics
```

Но это означает, что client code должен знать внутренности text backend.

Клиент будет знать:

- SDL_ttf text object semantics;
- rasterization concerns;
- cache behavior;
- primitive lifetime/usage pattern.

Это превращает backend implementation в API.

Вариант отклонён.

---

# 22. Решение, которое не подошло №9: `TextSystem` как большой God object

Рассматривалась идея:

```text
TextSystem
    measure
    draw
    invalidate
    lifecycle
    cache
    fonts
    rendering
    layout
```

Проблема — чрезмерная концентрация ответственности.

Старые layout analysis документы прямо предупреждают, что generic content measurement abstraction не должна превращаться в объект, который одновременно владеет:

- measure;
- arrange;
- invalidation;
- lifecycle;
- render;
- input.

Нужен narrow framework boundary.

---

# 23. Решение, которое не подошло №10: `TextRuntime` как просто wrapper над `TextPrimitive`

В текущей ветке был эксперимент:

```text
TextRuntime
    |
    +-- TextPrimitive
```

с:

```text
measure(...)
draw(...)
```

Но простая оболочка не решает главный architectural problem.

Если компонент всё равно должен напрямую знать, когда и как вызвать `TextRuntime`, leak просто меняет имя:

```text
Button -> TextRuntime
```

вместо:

```text
Button -> TextPrimitive
```

Это не настоящий boundary.

Поэтому сам по себе `TextRuntime` не является решением.

---

# 24. Решение, которое не подошло №11: LayoutSystem должен знать конкретные text-bearing components

Вариант:

```cpp
if (dynamic_cast<TextNode*>(&node)) ...
else if (dynamic_cast<Button*>(&node)) ...
else if (dynamic_cast<MenuItem*>(&node)) ...
else if (dynamic_cast<TabItem*>(&node)) ...
```

Проблема:

LayoutSystem начинает зависеть от конкретных components.

Каждый новый text-bearing component потребует изменения layout infrastructure.

Это нарушает generic infrastructure boundary.

С `StackPanelNode` ситуация другая: это специализированный layout node, и его type recognition непосредственно связано с layout semantics.

Text-bearing components — другой уровень.

---

# 25. Решение, которое не подошло №12: текст полностью убрать из framework infrastructure

Вариант:

```text
Custom component
    -> own TextPrimitive / own text rendering
```

Проблема:

Это заставляет клиента повторно реализовывать:

- text measurement;
- wrapping;
- font/raster handling;
- sharp logical/raster scaling;
- cache/resources.

Это противоречит исходной цели: layout и text behavior должны быть framework-managed.

Не подходит.

---

# 26. Что показали популярные framework'и

Поиск внешних архитектур показал три особенно полезных направления.

## Flutter

Flutter избегает text-aware inheritance hierarchy.

Текст представлен через отдельный render object, например `RenderParagraph`, который композиционно входит в render tree.

Идея:

```text
Button
  + text render object

CustomWidget
  + text render object
```

а не:

```text
Button : TextNode
```

Это показывает, что text behavior лучше выражать через composition, а не через множество text base classes.

---

## Qt

Qt выделяет text layout в отдельные объекты вроде `QTextLayout` и `QAbstractTextDocumentLayout`.

Главная идея:

```text
widget/component
    -> text representation
    -> text layout infrastructure
    -> actual rendering
```

Custom widget не обязан наследоваться от text widget, чтобы пользоваться text layout machinery.

Это хорошо соответствует требованию скрыть `TextPrimitive`.

---

## WPF

WPF разделяет controls/text properties и text formatting/layout engine.

Для custom text layout существует contract через `TextSource`, а layout выполняет `TextFormatter`.

Ключевая идея:

```text
component/control
    -> framework text contract
    -> text formatting/layout engine
```

а не:

```text
component
    -> internal text renderer
```

---

# 27. Вывод из внешних framework'ов

Популярные UI framework'и в разных формах избегают схемы:

```text
Button : TextNode
```

как универсального решения.

Они различают:

```text
text semantic/state
```

и:

```text
text layout/render implementation
```

Но конкретный механизм различается:

- composition;
- text layout object;
- text formatting callback/contract;
- framework property system.

Для текущего framework наиболее близки Qt/WPF-подобные идеи: компонент остаётся самостоятельным, а framework имеет внутренний text interpretation layer.

---

# 28. Проблема `TextNode`

`TextNode` сейчас является public class:

```text
TextNode : Node
```

и имеет:

```text
text
font
horizontalAlignment
verticalAlignment
color
```

При этом его текущая роль исторически — standalone text component и adapter между node lifecycle/geometry и text implementation.

Это не обязательно означает:

```text
TextNode = base class for anything with text
```

Напротив, попытка сделать его таким породила проблемы:

- Button получает чужой text contract;
- появляются варианты `TextNode`;
- растёт inheritance matrix;
- component semantics смешиваются.

Следовательно:

> `TextNode` не должен автоматически быть универсальной text capability base class.

---

# 29. Может ли любая component быть одновременно Node и TextNode?

Технически да:

```cpp
class Button : public TextNode
```

будет одновременно `Node` и `TextNode`, потому что `TextNode` уже наследует `Node`.

Но архитектурно это означает слишком сильное утверждение:

> Button полностью принимает semantic/visual contract TextNode.

Это может быть неверно.

Следовательно:

```text
Node + TextNode inheritance
```

не является достаточным универсальным решением.

---

# 30. Главная нерешённая задача

Нужно отделить:

```text
Text capability/semantic contract
```

от:

```text
TextNode concrete component
```

и от:

```text
TextPrimitive implementation
```

Три разные сущности:

```text
1. Text semantics
   Что framework умеет распознавать и интерпретировать.

2. TextNode
   Готовый стандартный Node-компонент,
   который предоставляет text semantics.

3. TextPrimitive
   Внутренняя SDL_ttf/raster/layout implementation.
```

Смешивать их нельзя.

---

# 31. Возможное правильное решение: framework semantic descriptor

Наиболее перспективная схема:

```text
Component
    owns arbitrary component-specific text state
        |
        v
framework-recognized semantic descriptor
        |
        v
Text infrastructure
        |
        v
TextPrimitive
```

Например внутреннее описание может содержать:

```text
text
font
horizontal alignment
vertical alignment
color
```

Но важно:

> Это не обязательно должно быть storage type компонента.

Компонент может иметь:

```cpp
std::string caption_;
TTF_Font *font_;
Color textColor_;
```

и только на framework boundary предоставить descriptor.

---

# 32. Почему descriptor не решает всё автоматически

Для measurement descriptor почти достаточен:

```text
Text descriptor
    -> Text measurement
    -> LayoutSize
```

Но rendering сложнее.

Например Button может иметь:

```text
press scale
special text rectangle
padding
border
state-dependent color
```

и framework не обязан знать эту component-specific presentation policy.

Поэтому text semantic descriptor не должен автоматически включать component-specific geometry.

Нужно разделять:

```text
Text semantics
```

и:

```text
component-specific text presentation geometry
```

---

# 33. Что делать с measurement

Текущий layout pipeline:

```text
LayoutSystem
    -> node.measureContent(content)
```

Это уже существующий framework-owned orchestration boundary.

Text measurement должен остаться частью этого framework-controlled process.

В идеале:

```text
LayoutSystem
    -> asks node/framework text contract
    -> TextPrimitive/Text layout implementation
    -> LayoutSize
```

а не:

```text
LayoutSystem
    -> knows SDL_ttf
```

и не:

```text
client
    -> manually measures text
```

---

# 34. Что делать с rendering

`NodeTree` сейчас делает:

```text
NodeTree::drawSubtree()
    -> node.draw(renderer)
    -> recurse into children
```

Это важный текущий contract.

Component `draw()` отвечает за собственную presentation.

Следовательно, не следует просто забирать весь text rendering из component draw и делать глобальный `NodeTree -> TextRuntime.draw()` без учёта component-specific geometry.

Особенно problematic для Button:

```text
currentScale_
press animation
presentation rectangle
```

являются component-specific.

Поэтому rendering integration должна учитывать существующий presentation boundary.

---

# 35. Что делать с custom component

Это ключевой тест любой будущей архитектуры.

Пусть клиент пишет:

```cpp
class ScoreNode : public ui::Node
{
public:
    void setText(std::string text);
    void setFont(TTF_Font *font);

protected:
    void draw(SDL_Renderer *renderer) override;
};
```

Он должен:

- знать text semantic API framework;
- знать `TTF_Font*`, если это public contract;
- НЕ знать `TextPrimitive`;
- НЕ знать `TextRuntime` implementation;
- НЕ вызывать LayoutSystem;
- НЕ запускать Measure/Arrange manually.

Это главный acceptance test.

---

# 36. Почему public `TTF_Font*` не является проблемой

Документация API прямо оставляет SDL/SDL_ttf частью public framework/client contract.

Следовательно:

```cpp
setFont(TTF_Font *font);
```

может быть public API.

Это не означает, что:

```cpp
TextPrimitive
TTF_Text
rasterFont_
```

также должны быть public.

Именно здесь проходит граница:

```text
public resource handle
    !=
public rendering implementation
```

---

# 37. Почему вариант «спрятать SDL entirely» тоже не нужен

Framework уже напрямую предоставляет SDL3/SDL3_ttf клиенту через CMake/public API.

Поэтому вводить:

```text
FontHandle
OpaqueFont
FrameworkFontResource
```

только ради сокрытия SDL не соответствует существующей архитектуре.

`TTF_Font*` может оставаться client-owned resource.

Framework должен только не захватывать ownership скрытно.

---

# 38. Ownership `TextPrimitive`

После исследования выяснено:

### Shared `NodeTree` primitive

Плюсы:

- меньше одновременно живых SDL_ttf stateful objects;
- общий text engine/workspace.

Минусы:

- primitive является только one-entry mutable cache, а не полноценным multi-text cache;
- возникают churn и resource recreation между разными компонентами;
- появился `Node::textPrimitive()` bridge;
- implementation detail поднялся в base Node.

### Per-component private primitive

Плюсы:

- чистый ownership boundary;
- primitive остаётся implementation detail component implementation;
- исторически это близко к исходной архитектуре.

Минусы:

- больше одновременно живущих SDL_ttf resources;
- raster font copy потенциально на каждый text-bearing component;
- может потребоваться общий lower-level cache позже.

Главный вывод:

> per-component ownership сам по себе не является проблемой.

Проблема начинается только тогда, когда `TextPrimitive` становится API, который должен знать client component author.

---

# 39. Почему shared runtime всё ещё может быть полезен

Новый logical/raster scaling делает `TextPrimitive` stateful.

Поэтому теоретически можно иметь:

```text
TextRuntime
    ├── shared TTF_TextEngine
    ├── raster cache
    └── TextPrimitive/workspace(s)
```

Но это полезно только как internal implementation detail.

Client component не должен знать, существует ли:

- один primitive;
- несколько primitives;
- cache map;
- per-renderer cache;
- pooled raster fonts.

Это может изменяться без изменения client API.

---

# 40. Почему `TextRuntime` не является решением сам по себе

Простой:

```text
TextRuntime
    -> TextPrimitive
```

ничего не решает, если component всё ещё обязан вызывать `TextRuntime` напрямую.

Тогда получаем:

```text
Button -> TextRuntime
```

вместо:

```text
Button -> TextPrimitive
```

Implementation всё ещё протекает наружу.

`TextRuntime` имеет смысл только **за framework semantic boundary**.

---

# 41. Текущее experimental состояние, которое не следует считать финальным

В текущей ветке был добавлен:

```text
src/core/text_runtime.hpp
src/core/text_runtime.cpp
```

но он пока является wrapper над `TextPrimitive`.

Также часть старого design всё ещё присутствует:

```text
NodeTree
    -> TextPrimitive

Node
    -> textPrimitive()

Button/MenuItem/TabItem/TextNode
    -> TextPrimitive
```

Поэтому это transition state, а не окончательная архитектура.

---

# 42. Что должна означать будущая text semantic API

Нужно стремиться к:

```text
PUBLIC

Node / component
    -> framework-recognized text semantic

INTERNAL

text semantic
    -> text measurement/render infrastructure
    -> TextPrimitive / SDL_ttf / raster cache
```

При этом component-specific text state не обязан быть одинаковым.

Например:

```text
Button
    text_
    font_
    textColor_
    fixed alignment

MenuItem
    text_
    font_
    textColor_
    own presentation rules

TextNode
    text_
    font_
    horizontal alignment
    vertical alignment
    color
```

Framework должен уметь получить то, что является framework-recognized text semantics, не заставляя все components иметь идентичный data model.

---

# 43. Промежуточный candidate: virtual semantic hook

Один из возможных вариантов:

```cpp
class Node
{
protected:
    virtual std::optional<TextProperties>
    getTextProperties() const noexcept;
};
```

Default:

```text
nullopt
```

`TextNode`, `Button`, `MenuItem`, `TabItem` и custom components могут override.

Преимущество:

- single inheritance;
- нет `Button : TextNode`;
- нет multiple inheritance;
- framework видит text semantics;
- component storage остаётся произвольным;
- `TextPrimitive` остаётся internal.

Но такой hook пока не следует считать окончательно принятым.

Причина: нужно решить, достаточно ли одного semantic descriptor или требуется ещё component-specific text geometry/presentation contract.

---

# 44. Почему `TextProperties` в `types.hpp` в текущем виде не должен автоматически стать public storage model

Если `TextProperties` объявляется как framework-recognized descriptor, он должен использоваться как **contract description**, а не как обязательный state holder.

Нельзя требовать:

```cpp
class Button {
    TextProperties text_;
};
```

потому что это снова возвращает universal property bag.

Правильнее концептуально:

```text
component-specific state
    -> semantic descriptor
```

а не:

```text
universal descriptor
    -> owns all component text state
```

---

# 45. Почему эта проблема не является аргументом за multiple inheritance

Multiple inheritance решает только discovery:

```text
Button : Node, TextContent
```

но не решает различия между набором свойств:

```text
Button != TextNode != MenuItem
```

Кроме того, это вводит новый архитектурный механизм в framework.

Если можно получить тот же result через один base Node semantic hook, это предпочтительнее.

---

# 46. Почему нельзя просто объявить «любая компонента имеет текст»

Framework не поддерживает arbitrary CSS-like property interpretation.

Поэтому generic Node с полем:

```cpp
std::string title_;
```

не должен автоматически становиться framework text object.

Framework должен иметь explicit, fixed, documented text contract.

То есть client должен знать:

```text
какие text semantics framework поддерживает
```

но не:

```text
как framework их реализует
```

---

# 47. Два принципиально честных варианта API

## Вариант A — TextPrimitive public contract

Клиент обязан использовать primitive.

Плюс:

- честно;
- просто объяснить.

Минус:

- internal implementation становится API;
- custom components знают rasterization/rendering details;
- framework больше не может свободно менять text backend.

Не рекомендуется.

---

## Вариант B — text semantic contract public, implementation private

Клиент знает:

```text
text
font
supported text semantics
```

Framework знает:

```text
measure
layout
rasterization
cache
rendering
```

Это соответствует общей architecture model и является предпочтительным направлением.

---

# 48. Acceptance criteria для окончательного решения

Будущее решение считается архитектурно правильным только если одновременно выполняются все условия.

### Client API

Custom component author:

```text
может создать Node
может задать framework-supported text semantics
не знает TextPrimitive
не знает TextRuntime internals
не знает text cache
не знает TTF_Text
```

### Framework

Framework:

```text
сам измеряет текст
сам инвалидирует layout при необходимых изменениях
сам применяет logical/raster scaling
сам управляет text rendering implementation
сам решает ownership/cache strategy
```

### Component independence

```text
Button не обязан быть TextNode
MenuItem не обязан быть TextNode
TabItem не обязан быть TextNode
CustomNode не обязан быть Button/TextNode
```

### Semantic variability

```text
разные components могут иметь разные наборы text-related properties
```

### Implementation opacity

```text
TextPrimitive можно полностью переработать
без изменения public component semantic API
```

---

# 49. Что окончательно отклонено

На текущем этапе считаются невыбранными/отклонёнными:

```text
X Button : TextNode как универсальное решение
X семейство TextNode variants
X Node + TextContent multiple inheritance
X generic ContentNode base
X обязательный TextProperties storage на каждом Node
X Node::textPrimitive()
X Node::drawText()/measureText() как public/protected primitive API
X LayoutSystem знание каждого text-bearing component
X TextPrimitive как public client contract
X TextRuntime как wrapper, который вызывают компоненты напрямую
X универсальный Content/Style bag для всех text semantics
```

---

# 50. Что считается доказанным

Следующие утверждения хорошо подтверждены исходниками/документацией:

1. `LayoutSystem` является framework-owned orchestration.
2. `Node` содержит generic framework-recognized state.
3. Concrete components владеют domain state.
4. `PanelNode`/`StackPanelNode` являются framework-defined specialized contracts.
5. `TextNode` является public framework concept.
6. `TextPrimitive` исторически задумывался как internal implementation.
7. `TextPrimitive` не должен быть `NodeTree`-public/client-facing service.
8. `TTF_Font*` может оставаться public contract.
9. `Button`, `MenuItem`, `TabItem` являются самостоятельными components.
10. Text-bearing components не обязаны иметь `TextNode` child.
11. Generic arbitrary property interpretation не является целью framework.
12. Shared property structure должна иметь concrete/stable common contract.
13. Logical/raster text scaling является реальной implementation responsibility `TextPrimitive`.

---

# 51. Что пока является архитектурным hypothesis, а не доказанным фактом

Пока окончательно не доказано:

1. Нужен ли `TextProperties` именно как public type или он должен быть internal descriptor.
2. Должен ли text semantic contract быть virtual hook `Node`.
3. Нужно ли text semantic discovery делать через `Node` или отдельную runtime mechanism.
4. Должно ли measurement text полностью переноситься из component `measureContent()` в framework text path.
5. Должен ли rendering текста происходить отдельным framework pass или через component-owned `draw()` при internal framework access.
6. Нужен ли один shared text runtime на `NodeTree`.
7. Нужен ли вообще `TextRuntime` как отдельный объект, если он останется только wrapper.
8. Можно ли оставить per-component private `TextPrimitive` и при этом предоставить custom component framework-managed text semantics другим способом.

---

# 52. Наиболее вероятная конечная модель

На текущем этапе наиболее сильная архитектурная гипотеза выглядит так:

```text
                         PUBLIC

Client component
      |
      | owns its own semantic state
      |
      | framework-recognized text contract
      v
Node / component semantic boundary
      |
      +--------------------------+
      |                          |
      v                          v
Layout infrastructure       Render infrastructure
      |                          |
      +------------+-------------+
                   |
                   v
            Internal text layer
                   |
                   v
             TextPrimitive
                   |
                   v
                SDL_ttf

                         INTERNAL
```

В этой модели:

- `TextNode` остаётся concrete standard component;
- `Button`, `MenuItem`, `TabItem` остаются самостоятельными components;
- custom `Node` может реализовать framework text contract;
- component-specific text state не обязан быть одинаковым;
- `TextPrimitive` не виден клиенту;
- framework остаётся ответственным за layout/rendering implementation;
- `TTF_Font*` остаётся частью существующего SDL-based public contract.

---

# 53. Главный принцип, который необходимо сохранить

Самая важная формулировка всей архитектуры:

> Клиент должен знать, **какие semantic properties framework понимает**, но не должен знать, **как framework реализует их обработку**.

Для layout это уже работает.

Для hit-testing это уже работает.

Для input это уже работает.

Для text эту же границу нужно довести до конца.

Плохой вариант:

```text
Client
  -> TextPrimitive
```

Правильная цель:

```text
Client
  -> framework-recognized text semantics
  -> framework
  -> TextPrimitive
```

---

# 54. Практическое следствие для текущей ветки

Не следует продолжать механически переносить `TextPrimitive` между папками/owners или переименовывать его в `TextRuntime`, пока не определён semantic contract.

Сначала нужно определить:

```text
1. как custom Node сообщает text semantics;
2. какие text semantics framework официально поддерживает;
3. какие свойства являются component-specific;
4. как framework получает text measurement input;
5. как framework получает text render geometry;
6. кто владеет actual TextPrimitive/cache implementation.
```

Только после этого следует менять:

```text
Node
TextNode
Button
MenuItem
TabItem
NodeTree
LayoutSystem
TextPrimitive
CMake
```

Большие `node_tree.cpp`/layout/runtime файлы должны меняться только после фиксации этого contract.

---

# 55. Final decision checkpoint

Пока окончательный implementation decision не зафиксирован.

Наиболее устойчивые решения:

```text
KEEP:
    public framework text semantics
    private TextPrimitive
    client-owned TTF_Font*
    framework-owned measurement/rendering orchestration
    independent Button/MenuItem/TabItem/TextNode

REMOVE / AVOID:
    Node::textPrimitive()
    public TextPrimitive contract
    Button : TextNode as generic text mechanism
    TextNode inheritance matrix
    multiple inheritance capability mixins
    universal text style/property bag
```

Основной unresolved point:

```text
Как именно framework должен получить text semantic description
от произвольного custom Node,
не превращая TextNode в base class всех text-bearing components
и не заставляя client code знать TextPrimitive.
```

Это является следующим архитектурным checkpoint.

---

# 56. Sources consulted

Основные источники проекта:

- `ui_framework/README.md`
- `ui_framework/docs/ARCHITECTURE.md`
- `ui_framework/docs/COMPONENT_DESIGN_GUIDE.md`
- `ui_framework/docs/FRAMEWORK_SCOPE.md`
- `ui_framework/docs/API_REFACTOR_CHECKLIST.md`
- `ui_framework/docs/PRIMITIVES_ROLE.md`
- `ui_framework/docs/PHASE5_SOURCE_AUDIT.md`
- `ui_framework/docs/PHASE5_COMPONENT_ARCHITECTURE_CHECKPOINT.md`
- `ui_framework/docs/LAYOUT_TEXT_ANALYSIS.md`
- `ui_framework/docs/LAYOUT_TEXT_PIPELINE_ANALYSIS.md`
- старый repository `Valeri-afk/ui-framework`
- исторические commits вокруг создания/refactoring `TextPrimitive` и `TextNode`

Также были рассмотрены внешние архитектурные подходы:

- Flutter rendering/text composition;
- Qt text layout abstractions;
- WPF text formatting/layout contracts.

---

# 57. Current recommendation

Не выбирать ownership `TextPrimitive` как первый вопрос.

Первый вопрос — **semantic contract**.

После того как contract определён, ownership становится внутренним implementation detail и может быть изменён без изменения client API.

Это и есть ключевой архитектурный критерий качества решения.
