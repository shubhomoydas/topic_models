#-----------------------------------------------
# Does the following:
#   - Ranks topics on the basis of entropy
#   - Computes KL-divergence between topics
#   - Outputs ranked vocabulary

fEntropy <- function(x) x*log(x)

runids <- c("lda-0","lda-1","lda-2")
ldaTypes <- c("LDA","TpT-LDA","GTpT-LDA")
id <- 3

epoch <- 4000
filenameprefix <- "taus"
runid <- runids[id]
infergroups <- ""
#infergroups <- "infergroups-"

inpath <- file.path("data/TwitterData/lda-runs","topics")
KLpath <- file.path("data/TwitterData/lda-runs","topics")
outpath <- file.path("data/TwitterData/lda-runs","topics")

infile1name <- paste(filenameprefix,"-",runid,"-1-",infergroups,as.character(epoch),".csv",sep="")
infile2name <- paste(filenameprefix,"-",runid,"-2-",infergroups,as.character(epoch),".csv",sep="")
KLfilename <- paste(filenameprefix,"-",runid,"-",infergroups,as.character(epoch),"-topics-stability-KL.csv",sep="")
outfilename <- paste(filenameprefix,"-",runid,"-",infergroups,as.character(epoch),"-topics-stability.csv",sep="")
heatmapfilename <- paste(filenameprefix,"-",runid,"-",infergroups,as.character(epoch),"-heat.png",sep="")

data1 <- read.csv(file.path(inpath,infile1name),header=F)
data2 <- read.csv(file.path(inpath,infile2name),header=F)

topics <- ncol(data1)-1

KL <- matrix(0,nrow=topics,ncol=topics)
for (i in 1:topics) {
  for (j in 1:topics) {
    kli <- sum(t(data1[,1+i])*log(data1[,1+i])) - sum(t(data1[,1+i])*log(data2[,1+j]))
    klj <- sum(t(data2[,1+j])*log(data2[,1+j])) - sum(t(data2[,1+j])*log(data1[,1+i]))
    KL[i,j] <- 0.5*(sum(kli+klj))
  }
}
pair <- c()
stab <- KL
for (i in 1:topics) {
  p <- order(stab[,i])[1]
  pair <- rbind(pair, c(i,p,KL[p,i]))
  stab[p,] <- 1000
}
pair <- pair[order(pair[,3]),]
stab <- matrix(0,nrow=topics,ncol=topics)
i <- 1
for (p_i in pair[,1]) {
  j <- 1
  for (p_j in pair[,2]) {
    stab[i,j] <- KL[p_j,p_i]
    j <- j + 1
  }
  i <- i + 1
}
#stab[1:5,1:5]

#col.mins <- apply(KL,2,min)
#stab <- KL[,order(col.mins)]
#row.mins <- apply(stab,1,min)
#stab <- stab[order(row.mins),]
#apply(stab,2,min)

graycolors <- gray.colors(256, start = 0.0, end = 1.0, gamma = 2.2, alpha = NULL)
#stab_heatmap <- 
#  heatmap(stab, Rowv=NA, Colv=NA, col = cm.colors(256), scale="column", margins=c(5,10))

xlabs <- rep("",topics); xlabs[1] <- "1"
xlabs[10*(1:(topics/10))] <- as.character(10*(1:(topics/10)))
ylabs <- rep("",topics); ylabs[1] <- "1"
ylabs[10*(1:(topics/10))] <- as.character(10*(1:(topics/10)))

#pdf(file=file.path(outpath,heatmapfilename))
#stab_heatmap <- 
#  heatmap(stab, Rowv=NA, Colv=NA, col = graycolors, scale="column", 
#          margins=c(3,3), labCol=xlabs, labRow=ylabs, 
#          xlab="topics (run 1)", ylab="topics(run 2)")
#dev.off()

require("lattice")
png(file=file.path(outpath,heatmapfilename))
levelplot(stab, cex.axis=1.5, cex.lab=1.5, 
          col.regions=graycolors, 
          #col.regions=colorRampPalette(c("blue", "yellow","red", "black")), 
          #at=seq(0,1.9,length=200), 
          xlab="topics (run 1)", ylab="topics (run 2)", main=ldaTypes[id])
dev.off()

#write.table(KL,file=file.path(outpath,KLfilename),col.names=F,sep=",",quote=F,row.names=F)
#write.table(stab,file=file.path(outpath,outfilename),col.names=F,sep=",",quote=F,row.names=F)

