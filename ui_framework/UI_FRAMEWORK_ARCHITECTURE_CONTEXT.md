# Архитектурный контекст и направление развития UI Framework

> **Статус:** architecture exploration / context record  
> **Ветка:** `fix/sharp-logical-text`  
> **Назначение:** зафиксировать причины текущей архитектуры, обнаруженные ограничения и направление дальнейшего исследования.  
> **Важно:** этот документ не является окончательным ADR и не фиксирует конкретное техническое решение. Его задача — сохранить архитектурный контекст и критерии, по которым будут оцениваться будущие варианты.

---

## 1. Зачем существует этот документ

В процессе развития `ui_framework` возникла проблема вокруг `TextNode` и `TextPrimitive`. При первоначальном рассмотрении проблема выглядела как отдельный вопрос text architecture: где хранить text state, кому принадлежит `TextPrimitive`, как разделить text semantics и text rendering implementation.

После анализа исходного кода и более общего обсуждения imperative/declarative UI стало очевидно, что `TextNode` является не изолированной проблемой, а первым серьёзным проявлением более общего свойства текущей архитектуры framework.

Текущая архитектура очень сильно скрывает от разработчика:

- lifecycle framework;
- scheduling;
- invalidation;
- структурные последствия изменения дерева;
- ownership/runtime bookkeeping;
- выполнение layout/input/render phases.

Это даёт сильные инварианты и делает framework предсказуемым со стороны runtime, но одновременно создаёт ограничения для custom components.

Цель этого документа — зафиксировать именно эту общую проблему до того, как начнётся переработка component model.

---

# 2. Текущее понимание природы framework

Наиболее точное описание текущего framework:

> **Retained-mode framework с декларативной семантикой, imperative C++ API и framework-owned execution/lifecycle.**

Это не React-подобный framework с virtual tree и diffing.

Это также не классический полностью imperative toolkit, в котором разработчик вручную управляет порядком layout/render/update.

Текущая модель находится между этими полюсами.

## 2.1. Retained mode

Framework хранит постоянное runtime tree:

```text
NodeTree
    ↓
persistent Node hierarchy
    ↓
ownership
identity
parent/child relations
live-node registry
lifecycle
layout state
input state
render state
```

Node не существует только как описание UI на текущий frame. Это живой объект framework runtime.

## 2.2. Imperative syntax

Разработчик пишет обычный C++:

```cpp
button.setText(...);
button.setPadding(...);
button.setVisible(...);

panel.addChild(...);
panel.removeChild(...);
```

То есть синтаксис и способ изменения state являются императивными.

## 2.3. Declarative semantics

При этом разработчик обычно не говорит framework:

```text
сначала сделай A
потом B
потом layout
потом hit-test
потом render
```

Он задаёт состояние или структурное отношение:

```text
Node visible = false
Node enabled = false
Button имеет такие параметры
Child является ребёнком Panel
```

А framework сам решает:

- когда обработать изменение;
- в какой фазе;
- в каком порядке;
- какие подсистемы затронуты;
- когда выполнить layout;
- когда выполнить input routing;
- когда сделать rendering;
- как сохранить ownership/lifecycle invariants.

Поэтому декларативность здесь находится прежде всего на уровне **control semantics**, а не синтаксиса.

---

# 3. Imperative vs declarative: важное разделение

В ходе обсуждения было важно перестать смешивать несколько разных понятий.

## 3.1. Imperative API != imperative runtime

Можно иметь:

```cpp
button.setPadding(...);
```

и при этом иметь:

```text
setPadding
    ↓
semantic state changes
    ↓
framework decides when/how to process it
```

Следовательно:

```text
imperative syntax
+
declarative semantics
+
framework-owned execution
```

является полноценной архитектурной моделью.

## 3.2. Declarative != diffing

Полноценный declarative framework не обязан использовать именно diffing.

Возможные механизмы определения изменений:

- tree diff / reconciliation;
- dependency tracking;
- signals;
- dirty propagation;
- observable properties;
- property metadata;
- explicit semantic notifications;
- query-time interpretation;
- full/partial recomputation;
- phase-driven evaluation.

Главный вопрос не в наличии diffing, а в наличии механизма, позволяющего framework узнать:

```text
что изменилось
↓
какие derived results теперь устарели
↓
какие framework phases должны быть запущены
```

## 3.3. Immediate Mode — отдельная модель

Immediate Mode GUI не следует смешивать с declarative retained mode.

В immediate mode код каждый frame описывает UI и одновременно управляет execution через порядок вызовов.

Это отдельная ось.

---

# 4. Что текущий framework уже делает декларативно

Важно, что декларативная семантика уже присутствует не только в layout.

## 4.1. `visible`, `enabled`, `focusable`, `capturable`

Например:

```cpp
node.setEnabled(false);
```

не означает:

```text
InputSystem.disableNode(node)
```

Оно означает:

```text
node.enabled = false
```

`InputSystem` сам интерпретирует это состояние.

То же касается:

- `visible`;
- `enabled`;
- `focusable`;
- `capturable`.

InputSystem при последующих проверках использует текущее состояние node.

Это query-time interpretation:

```text
semantic state
    ↓
InputSystem asks current state
    ↓
current interaction result
```

Для этого не требуется отдельный `hitTestInvalidation`.

## 4.2. Hit-testing

Текущий `NodeTree::hitTest()` обходит актуальное retained tree и проверяет:

- visibility;
- enabled;
- overflow/clipping;
- actual geometry;
- z-order;
- modal boundary.

Hit-test не использует отдельный кэш, который нужно постоянно инвалидировать.

Это один из наиболее простых и удачных участков текущей архитектуры.

## 4.3. Structural state

`PanelNode::addChild()` и `removeChild()` тоже имеют declarative semantics:

```text
"этот child теперь является ребёнком данного parent"
```

а не:

```text
"сделай вручную register + mount + layout + input bookkeeping"
```

`NodeTree` самостоятельно обеспечивает:

- ownership;
- live registry;
- parent relation;
- subtree registration/unregistration;
- mount/unmount;
- layout scheduling;
- mutation safety.

---

# 5. Сильная сторона текущей закрытости

Полная framework-owned execution model имеет реальные преимущества.

## 5.1. Framework может гарантировать invariants

Например `NodeTree` контролирует:

- live node registry;
- ownership;
- parent/child consistency;
- mutation queue;
- lifecycle order;
- subtree attach/detach;
- safe traversal.

## 5.2. Разработчик не обязан помнить внутренние операции

Например после:

```cpp
node.setPadding(...);
```

разработчик не должен отдельно помнить:

```text
invalidate layout
flush queue
rerun measurement
```

После:

```cpp
panel.addChild(child);
```

не нужно вручную:

```text
register node
set owner
mount subtree
schedule layout
```

## 5.3. Это особенно полезно для C++

C++ framework легко может разрушить свои инварианты, если пользователь свободно управляет:

- pointers;
- lifecycle;
- mutation timing;
- ownership;
- reentrant callbacks.

Поэтому желание скрыть runtime control является обоснованным.

---

# 6. Где закрытость начинает становиться проблемой

Главное ограничение текущей модели:

> **Framework знает framework semantics только там, где соответствующая semantics заранее встроена в framework-provided classes.**

Иными словами:

```text
framework property
    ↓
framework-defined type
    ↓
framework-aware mutation
    ↓
framework knows consequences
```

Это работает для:

- size;
- position;
- padding;
- border;
- overflow;
- visibility;
- enabled;
- focusability;
- capturability;
- и других заранее предусмотренных properties.

Но custom component может создать:

```cpp
class MyNode : public Node {
    std::string text_;
    float preferredSpacing_;
    CustomLayoutState state_;
};
```

и framework не знает:

```text
что эти properties существуют
↓
что они влияют на framework
↓
какие phases надо invalidiate
```

При этом текущая архитектура не даёт разработчику универсального:

```cpp
invalidateLayout();
invalidateRender();
invalidateInput();
```

Механизм invalidation намеренно закрыт.

Именно здесь возникает системное ограничение.

---

# 7. Почему `deferLayoutMutation()` является симптомом этой модели

Текущие setters имеют модель:

```cpp
void Node::setPadding(const Padding& padding)
{
    deferLayoutMutation(
        [padding](Node& node)
        {
            node.padding_ = sanitizePadding(padding);
        });
}
```

Аналогично для:

- size;
- position;
- min/max size;
- border;
- overflow;
- других layout-related properties.

Это не считается архитектурной ошибкой само по себе.

Наоборот, это правильно обеспечивает:

```text
property mutation
    ↓
safe deferred change
    ↓
layout scheduling
```

Проблема в другом:

> **Только framework-defined properties получают такой путь.**

Custom component не может естественным образом создать новое framework-aware property без либо:

1. наследования/композиции через framework type, который уже знает это property;
2. отдельного framework mechanism для объявления semantics;
3. уведомления framework;
4. более общего change-tracking system.

---

# 8. Как эта проблема породила текущую структуру компонентов

Текущая component model фактически стала такой:

```text
Base Node
    ↓
Developer component
```

а при необходимости framework capability:

```text
Base Node
    ↓
Framework-specific base
    ↓
Developer component
```

Например:

```text
Node
PanelNode
StackPanelNode
TextNode
...
```

Developer вынужден выбирать base class не только по семантике компонента, но и по вопросу:

> «Какие framework capabilities мне нужны, чтобы компонент вообще мог участвовать в системе?»

Это смешивает:

- semantic identity;
- framework capability;
- structural role;
- invalidation integration;
- lifecycle participation.

---

# 9. Structural tree problem

Вторая аналогичная проблема существует для детей.

Сейчас `PanelNode` имеет `children_`, а `NodeTree` является runtime authority.

`PanelNode::addChild()` работает примерно так:

```text
if no owner:
    mutate local children

if owner exists:
    delegate to NodeTree
```

Получается двойственность:

```text
PanelNode
    local structural mode

NodeTree
    framework-owned structural mode
```

Это работает, но выглядит архитектурно неестественно.

## Почему появился `PanelNode`

Потому что только `PanelNode` знает:

- как безопасно добавлять детей;
- как удалять;
- как сохранять parent invariant;
- как связывать children с NodeTree;
- как обеспечивать lifecycle/ownership.

Следствие:

```text
Node
    нельзя просто сделать container

PanelNode
    разрешено иметь children
```

Это означает, что inheritance становится не только semantic hierarchy, но ещё и способом получить право участвовать в framework structural protocol.

То есть:

```text
inheritance
    =
semantic identity
+
framework capability
```

Это считается потенциальным архитектурным smell.

---

# 10. Почему это ограничивает композицию

Произвольный custom component логически может хотеть:

```text
MyComponent
    has children
```

Но при текущем дизайне ему необходимо:

```text
inherit PanelNode
```

даже если его предметная семантика не является обычным `Panel`.

Это ограничивает:

- композицию;
- кастомные контейнеры;
- нестандартные child models;
- виртуализацию;
- специальные структурные отношения.

Это не означает, что `PanelNode` сам по себе плох.

Он решает реальную задачу.

Проблема в том, что **именно наследование от PanelNode становится authorization mechanism для structural participation**.

---

# 11. Почему `TextNode` стал первой серьёзной архитектурной проблемой

Text state должен участвовать одновременно в:

```text
text
+
font
+
wrapping
↓
measurement
↓
desired size
↓
parent layout
↓
rendering
```

Но если custom component просто хранит:

```cpp
std::string text_;
```

framework не знает, что изменение этого state делает предыдущий measurement stale.

И разработчик не может явно сказать:

```text
invalidate measurement
```

Поэтому естественно возникла необходимость сделать text state framework-aware.

Так появился `TextNode`.

Затем:

```text
TextNode
    ↓
TextPrimitive
    ↓
TextPrimitive ownership
    ↓
Node::textPrimitive()
```

И это уже стало утечкой implementation details.

Следствие:

> `TextNode` — не просто проблема text implementation. Это первый серьёзный симптом несовместимости между закрытой invalidation model и открытой custom component model.

---

# 12. Второй симптом — сложность добавления framework semantics

В текущей модели custom property можно создать сколько угодно:

```cpp
float foo_;
std::string bar_;
bool selectedByEngine_;
```

если framework не интересует их semantics.

Это нормально.

Но если property должна влиять на:

- layout;
- render;
- input;
- structure;
- lifecycle;

то возникает отдельная проблема.

Framework не должен пытаться «понимать все custom properties».

Большинство properties должны оставаться полностью локальными.

Нужен только способ сказать:

```text
"это custom state, но оно участвует в framework semantics"
```

и сделать это без передачи разработчику полного runtime control.

---

# 13. Ближайшие аналоги в существующих frameworks

При сравнении с существующими подходами были выделены несколько полезных моделей.

## 13.1. Классический imperative/retained framework

Qt/GTK-подобная модель:

```text
Framework owns:
    execution
    lifecycle
    traversal

Developer:
    implements hooks/contracts
    иногда явно сообщает об invalidation
```

Developer не запускает lifecycle сам, но framework предоставляет много точек участия.

## 13.2. Framework-managed property system

WPF показывает другой путь:

```text
property
    ↓
framework-known metadata
    ↓
property changes
    ↓
framework automatically knows affected phases
```

Например property metadata может выражать влияние на:

- measure;
- arrange;
- render;
- parent layout.

Важно:

> Framework не понимает произвольные свойства автоматически. Разработчик должен сделать property частью framework property system.

Это альтернативный путь к полному diffing.

## 13.3. Query-time semantics

Input/hit-test показывает третий путь:

```text
current state
    ↓
framework queries it
    ↓
result
```

Например `enabled` не обязательно требует invalidation hit-test.

## 13.4. Phase hooks

Custom component может предоставить:

```text
measure
arrange
draw
hitTest
lifecycle hooks
```

Framework решает:

```text
when
how often
in what order
```

а component определяет:

```text
how
```

Это классический framework extension contract.

---

# 14. Важное наблюдение: invalidation и lifecycle — разные проблемы

Не каждая subsystem требует одинакового change-tracking.

## Input/hit-test

Можно часто использовать:

```text
query current state
```

без отдельной invalidation.

Текущий `InputSystem` уже делает это.

## Lifecycle

Framework может сам знать:

```text
node attached
node detached
node shown
node hidden
```

и вызывать hooks.

Developer не обязан управлять lifecycle.

## Layout

Здесь проблема сложнее, потому что layout создаёт derived/cached state:

```text
text
    ↓
measure result
    ↓
parent desired size
    ↓
arrangement
```

Здесь нужна информация о staleness.

Это объясняет, почему text/layout являются значительно более сложной extension problem, чем hit-test.

---

# 15. Возможные направления развития

Рассматривались три крупных модели.

## Полностью imperative

Developer получает:

```text
invalidate
layout
update
tree notifications
```

Плюс:

- максимальная extensibility;
- custom component полностью контролирует свои dependencies.

Минусы:

- часть responsibility возвращается developer;
- легко сломать invariants;
- framework becomes protocol-heavy;
- runtime control становится менее централизованным.

Это считается слишком дорогим отходом от сильных сторон текущего framework.

## Полностью declarative/reactive

Framework получает общий механизм:

- observation;
- dependency tracking;
- signals;
- dirty propagation;
- reconciliation/diffing;
- или другой change-tracking system.

Плюсы:

- custom semantic state может участвовать в framework;
- developer почти не думает об invalidation.

Минусы:

- это уже большая самостоятельная подсистема;
- сложно и дорого реализовать в C++;
- нужно решать batching, dependencies, identity, scheduling, stale derived state;
- большая часть существующего runtime может потребовать серьёзной адаптации.

## Hybrid / controlled extensibility

Наиболее перспективный на данный момент вариант:

```text
Framework owns:
    lifecycle
    scheduling
    tree invariants
    layout orchestration
    input routing
    rendering
    consequences of semantic changes

Developer owns:
    component state
    custom behavior
    selected framework contracts
    selected semantic notifications
```

То есть:

```text
Developer controls WHAT
Framework controls WHEN
```

Это не переход к полностью imperative framework.

Это переход:

```text
closed framework
    ↓
controlled extensibility
```

с сохранением:

```text
framework-owned runtime
framework-owned lifecycle
framework-owned scheduling
framework-owned invariants
framework-owned orchestration
```

---

# 16. Семантические notifications

Рассматривалась идея, что framework не обязательно должен скрывать сам факт изменения каждого сложного domain.

Например custom component может сообщить:

```text
children structure changed
```

а framework дальше самостоятельно решает:

- registration;
- ownership;
- mount/unmount;
- layout;
- input reconciliation;
- rendering consequences.

Важно различать:

```text
notification:
    "произошёл факт"

command:
    "сделай конкретную runtime операцию"
```

Предпочтительнее первое.

Не рекомендуется универсальный escape hatch вроде:

```cpp
framework.invalidateEverything();
```

потому что он быстро превращается во вторую скрытую lifecycle API.

---

# 17. Structural notification

Для обычного framework-managed `addChild/removeChild` отдельная notification, скорее всего, не нужна, потому что framework уже знает о своей собственной mutation.

Но для custom structural source может быть полезен contract вида:

```text
children structure changed
```

Это особенно интересно для:

- custom containers;
- virtualized lists;
- dynamic child generation;
- нестандартных structural semantics.

Важно, чтобы developer не «ломал» внутренний `NodeTree` и потом просил framework его восстановить.

Notification должен сообщать об изменении **framework-visible semantic structure**, а не заменять безопасный mutation protocol.

---

# 18. Batching

Текущая модель хорошо работает с одиночными mutation, но плохо выражает границы более крупных semantic changes.

Например:

```text
20 state changes
+
5 structural changes
```

могут логически представлять одну операцию.

Желательный принцип для будущей модели:

```text
many semantic mutations
    ↓
transaction boundary
    ↓
framework processes consequences once
```

Это важно для:

- performance;
- consistency;
- custom component construction;
- structural updates;
- future property system;
- layout scheduling.

Batching не обязательно должен быть полностью публичной imperative API.

Он может быть следствием framework semantic transactions и notifications.

---

# 19. Component model, который сейчас пересматривается

Текущая модель слишком тесно связывает:

```text
Node inheritance
+
framework capabilities
+
semantic role
+
structural role
```

Целевая исследуемая модель должна, вероятно, различать:

## Base runtime object

```text
Node
```

Отвечает за:

- identity;
- parent;
- basic runtime state;
- basic lifecycle;
- base event integration.

## Structural capability

Например:

```text
can own framework-visible children
```

Не обязательно должен быть равен `PanelNode` как единственно возможный base class.

## Framework-recognized properties

Свойства, для которых framework знает:

- type;
- semantics;
- affected phases;
- mutation behavior;
- change propagation.

## Local component properties

Свойства, которые framework не знает и не должен знать.

## Framework participation contracts

Компонент может заявлять:

- measurement participation;
- layout participation;
- render participation;
- hit-test participation;
- lifecycle participation;
- structural participation;
- другие capability contracts.

При этом framework всё равно владеет временем выполнения.

---

# 20. Наследование не должно становиться authorization mechanism для всего

Сейчас выбор:

```text
Node
PanelNode
TextNode
...
```

частично означает:

> «Какую framework capability мне надо получить?»

Это приводит к ситуации, когда developer выбирает базовый класс не только по semantics компонента, но и по скрытым framework runtime requirements.

Желательная тенденция:

```text
base class
    ↓
минимальная runtime guarantee

contract/capability
    ↓
дополнительная framework participation
```

Это уменьшает coupling между semantic hierarchy и runtime mechanisms.

---

# 21. `PanelNode` не обязательно плох

Важно не сделать неправильный вывод.

`PanelNode` решает реальную задачу:

- children;
- ownership;
- traversal;
- structural mutation;
- tree invariants.

Проблема не в самом существовании `PanelNode`.

Проблема в:

```text
PanelNode
    =
единственный путь
получить structural framework capability
```

Это может быть слишком жёсткой extension boundary.

---

# 22. Layout как будущая точка расширения

Следующий возможный вопрос:

> Должен ли developer иметь возможность предоставить собственную layout logic?

У текущего framework уже существуют hooks:

- `measureContent`;
- `arrangeContent`.

Однако layout orchestration целиком остаётся framework-owned.

Возможное дальнейшее направление исследования:

```text
Framework controls:
    when measurement/arrangement happens

Component controls:
    how its own layout semantics are evaluated
```

Это не обязательно должно включать developer control над layout scheduler.

Layout extension лучше рассматривать отдельно после properties/structure/contracts, потому что он связан с derived state и invalidation.

---

# 23. Что НЕ является целью

На текущем этапе не ставится цель:

- сделать React-клон;
- добавить полноценный virtual DOM;
- обязательно внедрить diffing;
- создать generic reflection/property bag;
- отдать developer полный lifecycle control;
- сделать все properties observable;
- переписать весь runtime только ради декларативности.

Цель гораздо уже:

> **найти места, где закрытость framework уже причиняет реальную боль, и открыть именно эти extension points, не отдавая разработчику управление runtime.**

---

# 24. Критерии будущего extension model

Новая модель должна позволять:

### Custom property

Разработчик может иметь:

```text
arbitrary local state
```

и при необходимости объявить:

```text
framework-relevant semantics
```

без обязательного создания нового framework Node type.

### Custom structure

Разработчик может создавать компоненты с нестандартной внутренней структурой/композицией без обязательного наследования от заранее определённого `PanelNode`, если это действительно нужно.

### Custom lifecycle behavior

Разработчик может реализовать relevant lifecycle hooks.

### Custom input behavior

Разработчик может:

- custom hit-test;
- event handlers;
- focus/capture participation.

Framework остаётся владельцем routing/state.

### Custom layout

Разработчик может в определённых contract points предоставлять собственную layout logic.

Framework остаётся владельцем orchestration.

### Batching

Много semantic changes могут быть обработаны как одна logical update.

### Runtime invariants

Разработчик не получает прямого доступа к:

- live-node registry;
- internal mutation queue;
- scheduling internals;
- arbitrary lifecycle execution;
- raw framework traversal control.

---

# 25. Основная архитектурная гипотеза

На данный момент наиболее перспективной выглядит модель:

```text
                    Retained Runtime
                          │
                framework-owned execution
                          │
          ┌───────────────┼────────────────┐
          │               │                │
       Layout           Input           Render
          │               │                │
          └───────────────┼────────────────┘
                          │
                       Node
                          │
             ┌────────────┼────────────┐
             │            │            │
        local state   framework      hooks
                       contracts
             │            │            │
             │            └──────┬─────┘
             │                   │
             └───────────────┬───┘
                             │
                    semantic notifications
                             │
                             ▼
                   framework schedules work
```

Ключевой принцип:

> **Developer may describe semantic facts and provide behavior. Framework owns consequences and time.**

---

# 26. Что сейчас нужно исследовать дальше

Перед реализацией крупного рефактора требуется определить:

1. Какие properties являются действительно framework-level.
2. Какие properties должны оставаться полностью local.
3. Как custom property может стать framework-recognized.
4. Как custom component сообщает semantic change.
5. Какие изменения framework может обнаруживать самостоятельно.
6. Какие изменения требуют explicit notification.
7. Какую роль играют virtual methods.
8. Как выражать structural participation.
9. Как выражать layout participation.
10. Как устроить batching/transaction boundary.
11. Как сохранить текущие NodeTree invariants.
12. Как избежать превращения contracts в огромную lifecycle API.
13. Какие существующие subsystem assumptions придётся изменить.

---

# 27. Что считается проблемой текущей архитектуры, а что нет

## Считается проблемой

- framework-aware property должен находиться в framework-provided Node type;
- custom component не может естественно объявить новый framework-relevant state;
- child ownership/structural participation завязаны на `PanelNode`;
- component hierarchy и framework capability hierarchy смешиваются;
- `TextNode` появился как обход отсутствия общего text contract;
- `TextPrimitive` начал протекать из internal implementation;
- batching semantic changes недостаточно выражен;
- framework closure создаёт лишнюю стоимость расширения.

## Не считается проблемой автоматически

- наличие `NodeTree`;
- наличие mutation queue;
- framework-owned lifecycle;
- framework-owned input routing;
- query-time hit-test;
- virtual methods;
- retained tree;
- imperative C++ API.

Эти элементы сами по себе совместимы с хорошей retained declarative архитектурой.

---

# 28. Главная формулировка проблемы

Текущую проблему можно свести к одной фразе:

> **Framework умеет автоматически обрабатывать только те semantics, для которых заранее существует framework-owned representation.**

Это приводит к:

```text
new framework semantic
    ↓
new framework-aware property
    ↓
new framework Node/base class
    ↓
inheritance/composition pressure
    ↓
special-case component architecture
```

`TextNode` является первым большим примером этой цепочки.

---

# 29. Направление, которое сейчас исследуется

Не переход:

```text
Declarative → Imperative
```

и не переход:

```text
Retained → Immediate
```

а:

```text
Closed retained declarative semantics
            ↓
Controlled extensibility
```

с сохранением:

```text
framework-owned runtime
framework-owned lifecycle
framework-owned scheduling
framework-owned invariants
framework-owned orchestration
```

и добавлением:

```text
explicit semantic contracts
virtual participation hooks
semantic notifications
better component composition
possible property contracts
possible structural contracts
possible custom layout participation
batching/transaction semantics
```

---

# 30. Финальный рабочий тезис

Текущий framework уже доказал, что:

- framework-owned lifecycle работает;
- retained tree работает;
- centralized input routing работает;
- query-time hit-testing работает;
- framework-owned tree mutation работает;
- layout orchestration работает.

Первая серьёзная архитектурная проблема возникла там, где **custom component захотел добавить semantic state, который должен участвовать в framework-owned derived state**.

Это означает, что основной объект исследования на следующем этапе:

> **не TextNode, а extension model framework.**

Требуется найти такую модель contracts/properties/notifications/hooks, при которой:

```text
custom component
    +
custom state
    +
custom composition
    +
custom behavior
```

могут полноценно участвовать в framework, но при этом:

```text
framework
    сохраняет ownership
    lifecycle
    scheduling
    invariants
    orchestration
```

Это и есть главная цель дальнейшего архитектурного рефактора.

---

# 31. Открытые вопросы, намеренно не решённые этим документом

- Нужна ли property system и насколько она должна быть формализована?
- Нужен ли change-tracking механизм или достаточно semantic notifications?
- Нужен ли structural notification protocol?
- Нужен ли explicit batching/transaction API или это должна быть внутренняя модель?
- Должны ли contracts быть inheritance-based, virtual-method-based, composition-based или смешанными?
- Должен ли `PanelNode` оставаться отдельным type или стать реализацией более общего structural contract?
- Должна ли text semantics быть отдельным contract?
- Как именно должен выглядеть custom layout contract?
- Как далеко допустимо открывать lifecycle?
- Какие существующие NodeTree/Layout/Input assumptions придётся изменить?
- Насколько глубоко новый extension model потребует перестройки внутренней архитектуры?

До ответа на эти вопросы не следует считать текущий дизайн новым окончательным архитектурным решением.

---

# 32. Рабочий принцип для дальнейшего проектирования

При рассмотрении любого нового extension point задавать четыре вопроса:

```text
1. Что framework может узнать сам?

2. Какой semantic fact может знать только developer/component?

3. Что именно developer должен сообщить framework?

4. Какие последствия framework должен оставить полностью под своим контролем?
```

Если ответом становится:

```text
developer должен вручную управлять
lifecycle/scheduling/invalidation internals
```

— contract, вероятно, слишком открыт.

Если ответом становится:

```text
framework должен заранее иметь специальный Node
для любой новой semantics
```

— contract, вероятно, слишком закрыт.

Целевая область находится между этими двумя крайностями.
