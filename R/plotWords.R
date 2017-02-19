library(tm)
library(RColorBrewer)
library(wordcloud)

setwd("data/TwitterData/parsed/featurevectors")
data <- read.csv("vocab.csv", header=F, sep=",")

pal <- brewer.pal(8, "Dark2")
pal <- pal[-(1:2)]
#pdf("wordcloud-vocab.pdf")
png("wordcloud-vocab.png")
wordcloud(data[,2],data[,3], scale=c(8,.3),
          min.freq=2000, max.words=100,
          random.order=F, rot.per=.15, colors=pal, vfont=c("sans serif","plain"))
dev.off()
warnings()

##### Fonts

windowsFonts()
windowsFonts("mono")
windowsFonts(JP1 = windowsFont("MS Mincho"),
             JP2 = windowsFont("MS Gothic"),
             JP3 = windowsFont("Arial Unicode MS"))
