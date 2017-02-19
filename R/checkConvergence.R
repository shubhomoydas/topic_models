
# Analyze LDA model
inpath <- "experiments/lda"
outpath <- "experiments/lda"

topics <- 3

#taufile <- file.path(inpath, paste("synthetic_data_taus_T",as.character(topics),".csv",sep=""))
#thetafile <- file.path(inpath, paste("synthetic_data_thetas_T",as.character(topics),".csv",sep=""))

#ldaType <- "-lda0"
#ldaType <- "-lda1"
#ldaType <- "-lda2"
ldaType <- "-lda2g1"
ldaType <- "-lda2g2"
ldaType <- "-lda2g3"

taufile <- file.path(inpath, paste("synthetic_data_taus",ldaType,".csv",sep=""))

taus <- read.csv(file=taufile, header=F)

pchs <- c("o","+","*",".","^","~","$","#","=","-")
cols <- c("green","red","blue","black","orange","pink","cyan","magenta","gray","yellow")
drawlegend <- T

idxs <- unique(taus[,1])
fidx <- idxs[length(idxs)]
# get the last epoch data
tau <- taus[taus[,1]==fidx,]
f <- nrow(tau)
pdf(file.path(outpath,paste("FeatureDist_T",as.character(topics),ldaType,".pdf",sep="")))
plot(1:f,tau[,1+1],typ="l",lwd=2,col=cols[1],cex.axis=1.5,cex.lab=1.5,cex.main=2.0,
     main="Feature Distribution in Topics",ylab="Probability",xlab="Feature",
     ylim=c(0,max(tau[,1+(1:topics)])))
for (i in 2:topics) {
  lines(1:f,tau[,1+i],lwd=2,col=cols[i])
  #points(1:f,tau[,2+i],pch=pchs[1+i])
}
if (drawlegend == T) {
  legend(
    "topright",cex=1.20,
    legend=paste("Topic ",as.character(1:topics),sep=""),
    lwd=2,
    col=cols[1:topics]
  )
}
dev.off()

#----- Check entropy ----------
fEntropy <- function(x) x*log(x)
sum(fEntropy(tau[2]))
sum(fEntropy(tau[3]))
sum(fEntropy(tau[4]))

# KL-divergence between topics
KL <- matrix(0,nrow=topics,ncol=topics)
for (i in 1:(topics-1)) {
  for (j in (i+1):topics) {
    kli <- sum(t(tau[,1+i])*log(tau[,1+i])) - sum(t(tau[,1+i])*log(tau[,1+j]))
    klj <- sum(t(tau[,1+j])*log(tau[,1+j])) - sum(t(tau[,1+j])*log(tau[,1+i]))
    KL[i,j] <- 0.5*(sum(kli+klj))
  }
}
KL

n <- dim(thetas)[1]/3
uid <- 11
png(file.path(outpath,paste("Topics_User_",as.character(uid),".png",sep="")))
plot(1:n,typ="n",main=paste("Thetas for User",as.character(uid),sep=""),
     xlab="Iteration",ylab="P(theta)",
     cex.main=2.0,cex.lab=1.5,cex.axis=1.5,ylim=c(0,1))
for (i in 1:3) {
  theta <- thetas[i+3*(0:(n-1)),1+uid]
  lines(1:n,theta,col=cols[i],lwd=2)
}
legend(
  "topright",cex=1.20,
  legend=paste("Topic ",as.character(1:topics),sep=""),
  lwd=2,
  col=cols[1:topics]
)
dev.off()

n <- dim(taus)[1]/f
fid <- 1
ymax <- max(taus[,2:ncol(taus)])
#png(file.path(outpath,paste("Tau_Feature_",as.character(fid),".png",sep="")))
plot(1:n,typ="n",main=paste("Tau for Feature ",as.character(fid),sep=""),
     xlab="Iteration",ylab="P(tau)",
     cex.main=2.0,cex.lab=1.5,cex.axis=1.5,ylim=c(0,ymax))
for (i in 1:topics) {
  tau <- taus[fid+f*(0:(n-1)),1+i]
  lines(1:n,tau,col=cols[i],lwd=2)
}
legend(
  "topright",cex=1.20,
  legend=paste("Topic ",as.character(1:topics),sep=""),
  lwd=2,
  col=cols[1:topics]
)
#dev.off()

##============= Ranking ====================
#------ Load the base data for visualization

inpath <- "experiments/lda"
outpath <- "experiments/lda"

taufile <- file.path(inpath, paste("synthetic_data_taus.csv",sep=""))
thetafile <- file.path(inpath, paste("synthetic_data_thetas.csv",sep=""))

taus <- read.csv(file=taufile, header=F)
thetas <- read.csv(file=thetafile, header=F)

sum(thetas[,2:ncol(thetas)])
apply(thetas[,2:ncol(thetas)],1,sum)

#uid <- 28
#u <- apply(thetas[,(1+1+(uid-1)*300):(1+uid*300)],1,sum)
#u / sum(u)

topic_props <- as.matrix(apply(thetas[,2:ncol(thetas)],1,sum))
t_taus <- t(as.matrix(taus[,2:ncol(taus)]))

ranked <- t(rbind(1:ncol(t_taus),t_taus))
tcex <- c(0.8,1.0,1.2)
tcol <- c("green","red","blue")
data <- read.csv("sampledata/grouped-gmm-demo.csv")
pdf(file.path(outpath,paste("top_anoms_T",as.character(topics),".pdf",sep="")))
plot(data[,2], data[,3], xlab="x", ylab="y", pch="+", col="black")
for (topic in 1:topics) {
  ranked <- ranked[order(-ranked[,1+topic]),]
  topranks <- ranked[1:7,1]
  points(data[topranks,2], data[topranks,3], xlab="x", ylab="y", 
         pch="o", col=tcol[topic], cex=tcex[topic], lwd=2)
}
dev.off()

# Try composite ranking...
composite <- t(topic_props) %*% t_taus
compranked <- cbind(1:ncol(composite),t(composite))
compranked <- compranked[order(-compranked[,2]),]
topranks <- compranked[1:21,]
pdf(file.path(outpath,paste("top_anoms_T",as.character(topics),"_composite.pdf",sep="")))
plot(data[,2], data[,3], xlab="x", ylab="y", pch="+", col="black")
points(data[topranks,2], data[topranks,3], xlab="x", ylab="y", 
       pch="o", col="red", cex=1, lwd=2)
dev.off()

gmmdata <- read.csv("experiments/gmm/synthetic/ensembles/data-anomalies-grouped-bulk.csv",header=T)
lines <- strsplit(as.character(gmmdata$id),"-",fixed=FALSE)
gmmdata$lines <- as.numeric(unlist(lines)[2*(1:length(lines))])
pdf(file.path(outpath,"egmm_original_ranking.pdf"))
plot(data[,2], data[,3], xlab="x", ylab="y", pch="+", col="black")
topranks <- gmmdata$lines[1:21]
points(data[topranks,2], data[topranks,3], xlab="x", ylab="y", 
       pch="o", col="red", cex=1, lwd=2)
dev.off()
#------ End visualization
