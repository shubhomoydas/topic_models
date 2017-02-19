
inpath <- "data/TwitterData/lda-runs/lda-2-1-inferGroups"
rndinpath <- "data/TwitterData/lda-runs/random-groups"

data <- read.csv(file.path(inpath,"group-communication-counts.csv"),header=F)

data <- data[order(data[,2]),]

hist(data[,2])

ldaGroupCounts <- read.csv(file.path(inpath,"group-communication-counts.csv"),header=F)
rndGroupCounts <- read.csv(file.path(rndinpath,"random-groups2users-3-counts.csv"),header=F)

head(ldaGroupCounts)
head(rndGroupCounts)

median(ldaGroupCounts[,2])
median(rndGroupCounts[,2])

mean(ldaGroupCounts[,2])
mean(rndGroupCounts[,2])

wilcox.test(rndGroupCounts[,4],ldaGroupCounts[,4],paired = FALSE,alternative = c("greater"))

maxRange <- max(c(ldaGroupCounts[,2],rndGroupCounts[,2]))
h1 <- hist(rndGroupCounts[,2],breaks=seq(from=0,to=maxRange,length.out=100),plot=F)
h2 <- hist(ldaGroupCounts[,2],breaks=seq(from=0,to=maxRange,length.out=100),plot=F)
cols = c(rgb(0,0,1,1/4),rgb(1,0,0,1/4),rgb(0,1,1,1/4))
plot(h1, main=paste("Random vs LDA",measure,sep=""),
     col=cols[1], xlim=c(0,maxRange), xlab="grp coeff",ylab="val")
plot(h2,col=cols[2], xlim=c(0,maxRange), add=T)
legend("topright",cex=1.0,
       legend=c("Random","LDA Inferred"),
       lwd=3,
       col=cols)

