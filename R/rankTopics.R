#-----------------------------------------------
# Does the following:
#   - Ranks topics on the basis of entropy
#   - Computes KL-divergence between topics
#   - Outputs ranked vocabulary

fEntropy <- function(x) x*log(x)

outputKL <- F

epoch <- 2000
filenameprefix <- "taus"
runid <- "lda-2-2-inferGroups"

inpath <- file.path("data/TwitterData/lda-runs","topics")
outpath <- file.path("data/TwitterData/lda-runs","topics")

infilename <- paste(filenameprefix,"-",runid,"-",as.character(epoch),".csv",sep="")
outfilename <- paste(filenameprefix,"-",runid,"-",as.character(epoch),"-topics.csv",sep="")
outentropyfilename <- paste(filenameprefix,"-",runid,"-",as.character(epoch),"-entropy.csv",sep="")
outKLfilename <- paste(filenameprefix,"-",runid,"-",as.character(epoch),"-KL.csv",sep="")

vocabfile <- file.path("data/TwitterData/parsed/featurevectors","vocab.csv")

data <- read.csv(file.path(inpath,infilename),header=F)
vocab <- read.csv(vocabfile,header=F)

topics <- ncol(data)-1
topicsdf <- data.frame(rank=1:nrow(data))
topicnames <- c()
entropy <- rep(0,topics)
for (topic in 1:topics) {
  topicsdf <- cbind(topicsdf, vocab[order(-data[,1+topic]),2])
  topicnames <- c(topicnames, paste("Topic_",as.character(topic),sep=""))
  entropy[topic] <- -sum(fEntropy(data[,1+topic]))
}
#entropy[order(entropy)]
topicsdf <- topicsdf[,2:ncol(topicsdf)]
write.table(topicsdf,file=file.path(outpath,outfilename),col.names=topicnames,sep=",",quote=F,row.names=F)
write.table(cbind(0:(topics-1),entropy),file=file.path(outpath,outentropyfilename),col.names=F,sep=",",quote=F,row.names=F)

if (outputKL) {
  KL <- matrix(0,nrow=topics,ncol=topics)
  for (i in 1:topics) {
    for (j in 1:topics) {
      kli <- sum(t(data[,1+i])*log(data[,1+i])) - sum(t(data[,1+i])*log(data[,1+j]))
      klj <- sum(t(data[,1+j])*log(data[,1+j])) - sum(t(data[,1+j])*log(data[,1+i]))
      KL[i,j] <- 0.5*(sum(kli+klj))
    }
  }
  write.table(KL,file=file.path(outpath,outKLfilename),col.names=F,sep=",",quote=F,row.names=F)
}

