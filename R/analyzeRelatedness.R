
epoch <- 4000
filenameprefix <- "taus"
runseq <- 1
measures <- c("path","lesk","lin","res","vector")
runids <- c("lda-0-","lda-1-","lda-2-")
runids <- paste(runids,as.character(runseq),sep="")

plotHist = T
tTest = F
plotEntropy = T

inpath <- file.path("data/TwitterData/lda-runs","relatedness-all")
outpath <- file.path("data/TwitterData/lda-runs","analysis")
inpathentropy <- file.path("data/TwitterData/lda-runs","topics")

tukeyTest <- c()
wilcoxTest <- c()
medianTest <- c()
lmfit <- c()

#measure <- measures[1]
for (measure in measures) {
  
  alldata <- list()
  allentropy <- list()
  i <- 1
  for (runid in runids) {
    infilename <- paste(filenameprefix,"-",runid,"-",as.character(epoch),"-",measure,"-relatedness-summary.csv",sep="")
    outfilename <- paste(filenameprefix,"-",runid,"-",as.character(epoch),"-relatedness-analysis.csv",sep="")
    
    alldata[[i]] <- read.csv(file.path(inpath,infilename),header=F)
    
    inentropyfilename <- paste(filenameprefix,"-",runid,"-",as.character(epoch),"-entropy.csv",sep="")
    allentropy[[i]] <- read.csv(file.path(inpathentropy,inentropyfilename),header=F)
    
    if (plotEntropy) {
      outentropyfilename <- paste(filenameprefix,"-",runid,"-",as.character(epoch),"-",measure,"-entropy-relatedness.pdf",sep="")
      pdf(file.path(outpath,outentropyfilename))
      plot((allentropy[[i]])[,2], (alldata[[i]])[,2], main=paste(measure," relatedness vs entropy",sep=""),xlab="entropy",ylab="relatedness")
      dev.off()
    }
    
    i <- i + 1
  }
  cat("Loaded all data for ",measure,"\n")
  
  Relatedness <- c()
  algo <- c()
  for (i in 1:3) {
    lda <- alldata[[i]]
    Relatedness <- c(Relatedness, lda[,2])
    #Xi <- matrix(0,nrow=nrow(lda),ncol=3)
    #Xi[,i] <- 1
    #X <- rbind(X, Xi)
    algo <- c(algo, rep(i,nrow(lda)))
  }
  algo <- as.factor(algo)
  
  a1 <- aov(Relatedness ~ algo)
  summary(a1)
  
  coefmlm <- coef(summary(lm(Relatedness ~ algo)))
  lmfit <- rbind(lmfit, cbind(coefmlm, rep(measure,nrow(coefmlm))))
  
  wtestMeasure <- c()
  for (i in 1:2) {
    for (j in (i+1):2) {
      wtest <- wilcox.test(Relatedness[algo==i],Relatedness[algo==j],paired=F)
      wtestMeasure <- rbind(wtestMeasure, c(i,j,wtest$statistic,wtest$p.value))
    }
  }
  wilcoxTest <- rbind(wilcoxTest, cbind(wtestMeasure,rep(measure,nrow(wtestMeasure))))
  
  tukeyTestMeasure <- TukeyHSD(a1, "algo")$algo
  tukeyTest <- rbind(tukeyTest, cbind(tukeyTestMeasure,rep(measure,nrow(tukeyTestMeasure))))
  
  wtestMeasure <- c()
  for (i in 1:2) {
    for (j in (i+1):2) {
      x1 <- Relatedness[algo==i]
      x2 <- Relatedness[algo==j]
      m <- median(c(x1,x2))
      f11 <- sum(x1>m)
      f12 <- sum(x2>m)
      f21 <- sum(x1<=m)
      f22 <- sum(x2<=m)
      table <- matrix(c(f11,f12,f21,f22), nrow=2,ncol=2)  # 2x2 contingency table
      mtest <- chisq.test(table)
      mtest <- wilcox.test(Relatedness[algo==i],Relatedness[algo==j],paired=F)
      wtestMeasure <- rbind(wtestMeasure, c(i,j,mtest$statistic,mtest$p.value))
    }
  }
  medianTest <- rbind(medianTest, cbind(wtestMeasure,rep(measure,nrow(wtestMeasure))))
  
  if (tTest) {
    s0 <- Relatedness[algo==1]
    s1 <- Relatedness[algo==2]
    s2 <- Relatedness[algo==3]
    t.test(s0,s1,paired=F)
    t.test(s0,s2,paired=F)
  }
  
  if (plotHist) {
    maxRange <- max(Relatedness)
    hist0 <- hist(Relatedness[algo==1], breaks=seq(from=0,to=maxRange,length.out=100),plot=F)
    hist1 <- hist(Relatedness[algo==2], breaks=seq(from=0,to=maxRange,length.out=100),plot=F)
    #hist2 <- hist(Relatedness[algo==3], breaks=seq(from=0,to=maxRange,length.out=100),plot=F)
    
    cols = c(rgb(0,0,1,1/4),rgb(1,0,0,1/4),rgb(0,1,1,1/4))
    png(file.path(outpath,paste("hist-",runseq,"-",measure,".png",sep="")))
    plot(hist0, main=paste("",measure,sep=""),cex.main=1.5,cex.lab=1.5,
         col=cols[1], xlim=c(0,maxRange), xlab="Relatedness Score",ylab="frequency")
    plot(hist1,col=cols[2], xlim=c(0,maxRange), add=T)
    #plot(hist2,col=cols[3], xlim=c(0,maxRange), add=T)
    legend("topright",cex=1.5,
           legend=c("LDA","TpT-LDA"),
           #legend=c("SimpleLDA","TopicPerTweet","Grouped"),
           lwd=6,
           col=cols)
    dev.off()
  }
}

tmp <- cbind(row.names(tukeyTest),tukeyTest)
write.table(file=file.path(outpath,paste("tukey-",runseq,".csv",sep="")),
            tmp,col.names=c("comp","diff","upr","lwr","p-val","measure"),
            row.names=F,sep=",",quote=F)

tmp <- cbind(row.names(lmfit),lmfit)
colnames(tmp) <- c("coefficient",colnames(lmfit)[1:4],"measure")
write.table(file=file.path(outpath,paste("lmfit-",runseq,".csv",sep="")),
            tmp,col.names=T,
            row.names=F,sep=",",quote=F)

tmp <- wilcoxTest
colnames(tmp) <- c("algo_i","algo_j","W","p-val","measure")
write.table(file=file.path(outpath,paste("wilcox-",runseq,".csv",sep="")),
            tmp,col.names=T,
            row.names=F,sep=",",quote=F)

tmp <- medianTest
colnames(tmp) <- c("algo_i","algo_j","W","p-val","measure")
write.table(file=file.path(outpath,paste("median-",runseq,".csv",sep="")),
            tmp,col.names=T,
            row.names=F,sep=",",quote=F)
