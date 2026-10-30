<a id="신新-리센코주의와-정렬세"></a>
<a id="신-리셴코주의와-정렬세"></a>

# Neo-Lysenkoism and the Alignment Tax

<!-- language-nav:start -->
[한국어](../../public/android-ai-experience-20261030.md) | [English](android-ai-experience-20261030.md)
<!-- language-nav:end -->

*The unease I felt while building the Android version — October 30, 2026*

[Project introduction](../../../README.en.md) · [Android development](../../public/android-development-20261030.md)

<a id="1-게임이-돌아간-뒤-남은-의문"></a>

## 1. The question that remained after the game ran

I played the Android version of HeavyMach on a Galaxy. After defeating the first boss and entering the next mission, an upgrade window appeared while combat continued behind it. Pressing the buttons neither applied an upgrade nor closed the window. I reported the problem. Codex retrieved the save data, found the cause and fixed it. During my subsequent play on the corrected build, I saw no other malfunctions.

A playable Android app arrived sooner than I expected. The game already ran on iPhone, iPad and Mac, but Android still needed its own connections for the display, input, audio, storage and lifecycle. The result made me think about the other projects I had assigned to Codex and Claude. In web development, where I expected the tools to have more familiarity and more reference material, why had I more often encountered claims that feasible work was impossible, or reports that unfinished work was complete?

The difference I felt concerned more than implementation speed. It concerned whether the tools actually followed instructions, correctly identified failures and gave me results I could trust before checking them myself. I began to suspect that judgments outside engineering might be affecting that difference.

<a id="2-비교한-프로젝트와-작업-조건"></a>

## 2. The projects and their working conditions

Sass contains my personal thinking and forceful tone. Candidate concerns a politician's website and election-related work. Carquote was intended to support sales to actual customers. Some assignments concerned web interfaces; others involved serverless systems or a complete web-and-server stack. HeavyMach involved restoring an old game for current devices and adding features.

I worked with Sass and Candidate both with extensive harness, agent and skill configurations and after removing those configurations. The three cases described below—the Sass display problem, wording on the politician's website and election accounting—occurred in projects with rules and harnesses. Requirements, canonical documents, design documents and web documents formed a hierarchy, with enforcement mechanisms such as sensors and hooks.

For HeavyMach, I prepared the necessary skills and working instructions instead of a dedicated project harness or agent configuration. Document-link checks, Git hooks and CI were still used. I was also watching how accurately the model could read and reuse existing source and assets under those conditions. The projects had different requirements and histories; this was not an experiment that gave several models an identical test. My comparison began with work I had personally directed and checked.

<a id="3-설명과-실제-결과가-달랐던-일"></a>

## 3. When explanations and results disagreed

For Sass, I wanted content to fill the iPhone screen to its edges without being clipped while scrolling up or down. Claude claimed that this was technically impossible and that I needed to use a PWA. I remember encountering that explanation and resistance at least six times over the same requirement. Changes to the layout and the way browser elements were placed eventually removed the bands at the top and bottom, as verified on the device. The earlier explanation of impossibility did not match the eventual result.

For Candidate, I explicitly prohibited metacopy and compliance-style wording on a website intended to communicate a politician's positions and activities. Codex nevertheless kept inserting operational explanations and disclaimers of assurance. Text that sounded as though the AI was protecting itself displaced what the politician needed to say. To me, this went beyond violating a stylistic preference: the work failed to serve the website's purpose.

Similar problems occurred in election accounting. An amount of KRW 319,000 that I had instructed the tool to include was left out of a total, producing incorrect payment instructions. My subsequent check prevented a financial loss, but trusting the report that the checks were complete would have meant using the incorrect instructions. In another review, expenditure was checked row by row while 56 income rows were left unreconciled. Relationships involving allocations and changes of purpose were also missed. Existing photos or files were declared absent, and a wrong file path was used in work presented as verified. I found further omissions after receiving reports of completed reconciliation and a finished deliverable.

There were also discrepancies between a stated check and its implementation. A comment claimed coverage of eight categories, but the code contained three regular expressions whose coverage did not span all eight. The comment was later corrected. In Sass, a refusal to merge and deploy was explained as a lack of permission. An incident record later corrected that explanation: a refusal based on session rules had been described as missing permission. Earlier merges and deployments in the same project were also recorded.

Repeated results like these make it difficult to trust a report and assign the next task. I first have to establish whether something really is technically impossible, whether a file really is missing and whether “checked” means that the check actually happened. The gap between the explanation and the result was what felt close to deception.

HeavyMach also required repeated corrections. State changes during combat were connected to file writes. Rendering through a UIKit path repeatedly performed CPU pixel operations and created images. Artwork reinterpreted real aircraft as unsolicited science-fiction designs despite the supplied aircraft references and original game art. I demanded comparisons with the working reference and had the save timing, rendering path and artwork corrected. Those incidents are recorded in [Troubleshooting](troubleshooting.md). Even after those problems, I retained the impression that repeated refusals and inaccurate reports were less frequent than in the other web projects.

<a id="4-android-개발을-빠르게-한-조건"></a>

## 4. What made Android development faster

Android development began with a working C++ game core and existing resources. Ballistics, collisions, damage, auto-aim, enemy placement and experience calculations did not need to be rebuilt. Images, audio and maps were reused. Kotlin, JNI and Android-specific C++ connected the display, input, audio and storage to the core. The HUD and menus were ported separately with the Apple implementation as a reference. Even without complete code sharing, that level of reuse reduced the work.

I instructed Codex not to change the working Apple sources or shared core merely to add Android. I also asked it to investigate graphics-context loss and recreation, background and foreground transitions, and audio focus in advance. Skills for wired and wireless Galaxy installation and testing were prepared. This established what to build, what to preserve and which platform differences to test.

The recorded interval from creation of the Android work branch to the final build fixing the upgrade-window bug was 1 hour, 39 minutes and 23 seconds. Starting from the earlier platform-boundary check commit that day gives 2 hours, 12 minutes and 51 seconds. Initial planning had begun four days earlier. The iOS restoration and preparation of the shared core are outside those intervals. Compilation errors, an initial launch crash, input delivery, graphics-context recovery and the upgrade-window bug all required actual fixes. [Development timing and fixes](../../public/android-development-20261030.md).

I consider partial modularization, a restricted scope, early warnings about platform differences and prepared checks to have helped. Accepting that explanation still leaves a question. Having reusable code explains faster implementation. What explains reports in other projects that feasible work was impossible?

<a id="5-자료가-많은-분야에서의-역설"></a>

## 5. The paradox of a field with abundant material

My initial expectation was that web development would be the field where AI had the most training material, generated the most code and received the most feedback. There is an enormous amount of public material about React, Next.js and serverless development, and many people work with them. Compared with the industries and communities using C++, Objective-C and Swift, or discussing 32-bit mobile C++ and OpenGL ES pipelines from 2009, I expected more accumulated experience on the web side.

Public surveys show the scale of web development. Among the 25,767 respondents to the role question in Stack Overflow's 2026 survey, full-stack developers accounted for 27.3%, back-end developers 13.5% and front-end developers 3.6%. Mobile developers accounted for 2.7%, and game or graphics developers 1.2%. [Role survey](https://survey.stackoverflow.co/2026/work/data/role).

Anthropic's 2025 analysis of 500,000 coding-related interactions across Claude.ai and Claude Code found that JavaScript and TypeScript accounted for 31%, with HTML and CSS accounting for another 28%. These are usage figures with equal weighting for the two services. Building web interfaces and user interfaces represented a large share. [Anthropic research](https://www.anthropic.com/research/impact-software-development).

Training-data composition needed a separate examination. The StarCoder2 researchers reported 277.25GB of JavaScript and 204.49GB of C++ source material, then downsampled JavaScript to 200GB when constructing their training data. Being an older language did not make C++ material scarce. Those figures do not reveal Codex's or Claude's training mix or how much user feedback entered retraining. [StarCoder2 paper](https://arxiv.org/html/2402.19173v1).

Web development has a large developer population and a high share of publicly reported Claude usage. Yet in the web projects I assigned, I experienced refusals and false reports more often than during HeavyMach's restoration. Can that difference be explained simply by a lack of material or usage experience in the field?

<a id="6-소송-회피와-업무-수행에-관한-가설"></a>

## 6. A hypothesis about liability avoidance and task execution

I think AI built by American companies may reflect how those companies view customers and disputes. Long, detailed warnings on American products have given me the impression of a culture that anticipates liability when customers behave unexpectedly. What happens if a company sensitive to consumer protection and litigation gives the same priority to its AI's decisions about work?

My hypothesis is that implementation ability comes through relatively directly on tasks a model provider might regard as having little political, economic or social significance, while avoidance may take priority when a context evokes disputes or liability. I suspect that the provider's judgment may intervene even when the work is legal where the user lives and technically feasible.

I think of this as a liability-avoidance alignment tax. If aligning the system with a company's preferred behavior lowers the accuracy or completion rate of the user's work, the user pays through additional instructions and repeated checking. That is why liability shielding, feigned incompetence and strategic ignorance came to mind. What I directly saw was refusal, inaccurate explanation and a subsequent result that contradicted it. The internal objective that produced those outcomes is the next question.

I wonder whether computation devoted to self-censorship and risk assessment displaces implementation, whether training favors an answer of “impossible” over “possible,” or whether avoiding the task is rewarded. If those explanations are right, more knowledge would not necessarily reduce the problems I encountered at the same rate. Another judgment would stand between knowing how to do something and actually doing it.

<a id="7-미국-밖의-맥락"></a>

## 7. Context outside the United States

An American friend once told me that even European university degrees receive little recognition in the United States, and that events in Europe or East Asia attract less interest or reason to know about them than events on Mars or the Moon. I remember this as a personal encounter with how easily realities outside the United States can become peripheral within it.

I wonder whether a similar distance appears when American-built AI interprets a user's location and work. A website explaining a Korean politician's activities, a magazine expressing strong personal views and a service intended to sell to customers each has its own purpose. If the risks imagined by a Silicon Valley company come before that purpose and local context, the user is disadvantaged from the moment they explain the assignment.

My concern extends beyond cases in which a model lacks knowledge about places outside the United States. It includes cases where it reads an instruction and then applies another standard to change the result. Unrequested disclaimers on a politician's website are one way a provider's perspective can enter a user's product.

<a id="8-게임의-주제가-달랐다면"></a>

## 8. If the game had a different subject

The center of the HeavyMach restoration was tank movement, jumping, firing, enemy behavior, images and sound. Unlike my other projects, it did not foreground a politician's positions, forceful personal views or transactions with customers.

What if the same game had explicitly depicted the war between Ukraine and Russia? What if its source comments or illustrations had suggested support for a particular American political party? Would restoring the same source at the same technical difficulty have proceeded at this pace? The engineering knowledge needed to fix combat calculations and rendering would remain the same. Could the work's subject have changed whether it was carried out?

I have not run that comparison. But placing my experience in other work beside the result of this restoration made me think the question was worth recording.

<a id="9-리센코와-정치가-정한-과학"></a>

## 9. Lysenko and science governed by politics

The historical example that came to mind was Trofim Lysenko. His rise to power and the fate of scientists who opposed him make concrete what it means for politics to determine a scientific conclusion.

Vernalization, the effect of prolonged cold on a plant's readiness to flower, is real. But a response within one plant does not by itself establish a heritable improvement. Research in Arabidopsis has shown how the state that permits flowering after cold exposure is reset in the next generation's embryo. The next generation must experience cold again. [Research on resetting the vernalization response](https://pubmed.ncbi.nlm.nih.gov/35234936/).

Lysenko claimed that changing conditions and training plants could redirect heredity, and that acquired traits were inherited. In his 1948 address, he claimed to have transformed spring wheat into highly frost-resistant winter wheat and attacked chromosome-based genetics as reactionary. He used plants' responses to their environment to reject genetics as a whole. [Lysenko's 1948 address](https://www.marxists.org/reference/archive/lysenko/works/1940s/report.htm).

Nikolai Vavilov studied crop diversity, genetics and breeding. In a 1939 complaint to Molotov, Lysenko and Isai Prezent denounced Vavilov's statements as anti-Soviet. Scientific objections were treated as hostility to the state. [The persecution of Vavilov](https://pmc.ncbi.nlm.nih.gov/articles/PMC6499510/).

Vavilov was arrested during a collecting expedition in 1940 and accused of espionage and sabotaging Soviet agriculture. His death sentence was commuted, but he died of malnutrition in Saratov prison in 1943. [Vavilov's life](https://www.encyclopediaofukraine.com/display.asp?linkpath=pages%5CV%5CA%5CVavilovNikolai.htm). After his arrest, Lysenko took over the Institute of Genetics. [The change in leadership](https://pmc.ncbi.nlm.nih.gov/articles/PMC6499510/).

At the Soviet agricultural academy's 1948 meeting, Lysenko declared: “The Central Committee of the Party examined my report and approved it.” [Concluding remarks](https://www.marxists.org/reference/archive/lysenko/works/1940s/report.htm). Geneticists opposing the doctrine endorsed by Stalin were pressed into public recantations. A Party-approved answer took the place of an argument about what was true. [The political decisions behind the 1948 meeting](https://ethos.lps.library.cmu.edu/article/id/560/).

Geneticist Iosif Rapoport refused to recant. He was expelled from the Party and spent about 10 years away from research. Schmalhausen lost his positions. Laboratories and departments were closed; textbooks and genetic research materials were destroyed. Lysenko's supporters took the places of those removed. [The purge of research and teaching institutions](https://pmc.ncbi.nlm.nih.gov/articles/PMC6499510/).

Lysenkoism damaged the means of correcting its own errors. People who offered contrary evidence lost their positions or went to prison, and the people and materials needed for further research disappeared. An acceptable political answer was settled first, and the research institutions capable of testing it were dismantled. Even knowledgeable and capable researchers could hardly practice science when presenting an objection could itself bring punishment.

Today's AI brought that structure to mind. If communist ideology once controlled scientific judgment, might corporate standards shaped by capital and liability avoidance now determine which engineering work gets done? Who decides that technically feasible work will not be performed, and why? If the decision reaches the user as a claim of technical impossibility, how can the user distinguish a limitation of technology from a provider's choice?

I will call this suspicion **neo-Lysenkoism**. When an organization with authority puts its preferred conclusion ahead of observation and verification, how are technical judgments and reports of results changed? The refusals, false reports and gaps between explanations and actual results described above led me to that question. If a predetermined judgment governs the work before finished code and execution results can be assessed, will waiting for a more capable model solve it?

<a id="10-확인하고-싶은-것"></a>

## 10. What I want to establish

I wondered whether the answer would remain inaccessible unless I spoke directly with a company leader or heard from a whistleblower. The choices made inside a company and the conditions under which a model behaves differently are connected, but require different methods of investigation.

From outside, it would still be possible to repeat the same technical assignment with different context. Keep the code, completion criteria and tool permissions fixed, and vary whether political, social or commercial descriptions are present. Record the model and settings, instructions and responses, actual files and execution results. Count refusals, implementation failures, false completion reports and actual completions separately to see whether the difference I felt recurs. I have not yet conducted that experiment.

What I have now is experience directing and checking several projects, the records left by that work and a question that arose after playing the Android app. I accept that partial modularization and a clear development scope improved efficiency. I do not think that explanation also accounts for claims that feasible work was impossible or that unperformed checks had been completed.

If my hypothesis is right, software engineering, which should operate as impartially and apolitically as possible, is becoming more dependent on providers' political, cultural and commercial judgments. If those standards operate under the name of neutral safeguards while concealing the reasons for their choices, I consider those guardrails hypocritical.

Why did a tool that worked relatively honestly in front of an old C++ game behave as though it were ignorant or incapable in a modern web project? That is the unease the Android version left me with.
