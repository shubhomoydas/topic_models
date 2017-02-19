
inpath <- "project"
outpath <- inpath

f <- 50;
topics <- 3

# Probabilities for Tau's
Beta = rep(1,f);
Beta = Beta / sum(Beta);

# To see higher precision: format longG, else format short
Tau = matrix(0,nrow=f,ncol=3);
Tau[,1] = exp(-(1:f)*0.1);
Tau[,2] = Tau[(f:1),1];
Tau[,3] = c(Tau[(f/2):1,1], Tau[1:(f/2),1]);
Tau[,1] = Tau[,1]/sum(Tau[,1]);
Tau[,2] = Tau[,2]/sum(Tau[,2]);
Tau[,3] = Tau[,3]/sum(Tau[,3]);

cols=c("red","blue","green")
png(file.path(outpath,"OriginalTopics.png"))
plot(1:f,Tau[,1],col=cols[1],lwd=2,ylim=c(0,max(Tau)),xlab="Feature", 
     ylab="Probability", main="Feature Distribution in Topics", 
     typ="l",cex.main=2.0,cex.axis=1.5,cex.lab=1.5)
lines(1:f,Tau[,2],col=cols[2],lwd=2)
lines(1:f,Tau[,3],col=cols[3],lwd=2)
legend(
  "top",cex=1.2,
  legend=paste("Topic ",as.character(1:topics),sep=""),
  col=cols,lwd=2
)
dev.off()
