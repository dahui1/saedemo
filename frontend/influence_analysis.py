class asker():

    def __init__(self, client):
        self.a=int()
        self.tname=[]
        self.read_topic()
        self.client = client

    def ask_pie(self,id):
        pub=self.client.influence_search_by_author("",id)
        result1=pub.influence
        score=[{}for x in range(300)]
        for i in range(300):
            score[i]['score']=0
            score[i]['topic']=0
        for i in result1:
            score[i.topic]['score']+=i.score
            score[i.topic]['topic']=i.topic
        score.sort(key=lambda x:x['score'],reverse=True);
        top_pie=[]
        for i in range(0,5):
            top_id=score[i]['topic']
            tot=score[i]['score']
            top_pie_t={}
            tmp=self.tname[int(top_id)].split('/')
            top_pie_t['label']=tmp[0]
            top_pie_t['value']=tot
            top_pie.append(top_pie_t)
        return top_pie

    def read_topic(self):
        fileHandle = open ('topic2name.txt')
        fileList = fileHandle.readlines()
        i=0
        for fileLine in fileList:
            words=fileLine.split('\t')
            tmp=words[1].split('\n')
            self.tname.append(tmp[0])
        fileHandle.close()

    def ask(self, id):
        a=int()
        a=id
        pub=self.client.influence_search_by_author("",a)
        result1=pub.influence
        result2=pub.influenced_by
        score=[{}for x in range(300)]
        for i in range(300):
            score[i]['score']=0
            score[i]['topic']=0
        influence=[[]for x in range(300)]
        influenced_by=[[]for x in range(300)]
        num_influence=[0 for x in range(300)]
        num_influenced_by=[0 for x in range(300)]
        for i in result1:
            influence[i.topic].append(i)
            score[i.topic]['score']+=i.score
            score[i.topic]['topic']=i.topic
            num_influence[i.topic]+=1
        score.sort(key=lambda x:x['score'],reverse=True);
        for i in result2:
            influenced_by[i.topic].append(i)
            num_influenced_by[i.topic]+=1
        top_list=[]
        for i in range(0, 5):
            top_id=score[i]['topic']
            topics = {}
            topics['influencers']=[]
            topics['influencees']=[]
            topics['topic']=self.tname[int(top_id)]
            influence[top_id].sort(key=lambda x:x.score,reverse=True)
            influenced_by[top_id].sort(key=lambda x:x.score,reverse=True)
            num_1=min(5,num_influence[top_id])
            num_2=min(5,num_influenced_by[top_id])
            for j in range(0,num_1):
                tmp_l=[]
                result=self.client.author_search_by_id("",[influence[top_id][j].id])
                tmp_l.append(result.entity[0].title)
                tmp_l.append(influence[top_id][j].score)
		url="../"
		url+=str(result.entity[0].id)
		url+="/influence"
		tmp_l.append(url)
                tmp_t=tuple(tmp_l)
                topics['influencees'].append(tmp_t)
            for j in range(0,num_2):
                if (influenced_by[top_id][j].id==a):
                    continue
                tmp_l=[]
                result=self.client.author_search_by_id("",[influenced_by[top_id][j].id])
                tmp_l.append(result.entity[0].title)
                tmp_l.append(influenced_by[top_id][j].score)
                url="../"
		url+=str(result.entity[0].id)
		url+="/influence"
		tmp_l.append(url)
                tmp_t=tuple(tmp_l)
                topics['influencers'].append(tmp_t)
            top_list.append(topics)
        result=self.client.author_search_by_id("",[a])
        final=dict(topics=top_list, name=result.entity[0].title)
        return final

class asker_t():

    def __init__(self, client):
        self.a=int()
        self.client = client

    def ask(self, id):
        self.a=id
        pub_result=self.client.pub_search_by_author("",self.a)
        pub=[[]for x in range(3000)]
        num=[0 for x in range(3000)]
        cita=[0 for x in range(3000)]
        for item in pub_result.entity:
            year=item.stat[0].value
            if year==2013:
                continue
            pub[year].append(item)
            num[year]+=1
            cita[year]+=item.stat[2].value
        trend=[]
        for i in range(1000,3000):
            if num[i]>0:
                pub[i].sort(key=lambda x:x.stat[2].value,reverse=True);
                tmp={}
                tmp['date']=str(i)
                tmp['value']=num[i]
                #tmp['value']=cita[i]
                tmp['pap1']=pub[i][0].title
		result=self.client.author_search_by_id("",pub[i][0].related_entity[0].id)
		#author=result.entity[0].title
		author=', '.join([e.title for e in result.entity])
		#for j in range(1,result.total_count):
                    #author+=', '
		    #author+=result.entity[j].title
                tmp['au1']=author
                tmp['cit1']=pub[i][0].stat[2].value
		tmp['pap2']=0
                tmp['au2']=""
                tmp['cit2']=0
                if num[i]>1:
                    tmp['pap2']=pub[i][1].title
                    result=self.client.author_search_by_id("",pub[i][1].related_entity[0].id)
		    author=result.entity[0].title
		    for j in range(1,result.total_count):
                        author+=', '
			author+=result.entity[j].title
		    tmp['au2']=author
                    tmp['cit2']=pub[i][1].stat[2].value
                trend.append(tmp)
        return trend

class asker_table():

    def __init__(self, client):
        self.client = client
        self.N=1231987
        N=1231987
        self.M=10380000
        M=10380000
        self.first=[-1 for x in range(N)]
        self.nex=[0 for x in range(M)]
        self.en=[0 for x in range(M)]
        self.dis=[0 for x in range(M)]
        self.peo=[0 for x in range(N)]
        self.num=[0 for x in range(N)]
        self.au=[0 for x in range(N)]
        self.has=[0 for x in range(14377300)]
        self.out=[0 for x in range(N)]
        fileHandle = open ( 'au.txt')
        fileList=fileHandle.readlines()
        i=0
        for fileLine in fileList:
            words=fileLine.split(' ')
            o_id=int(words[0])
            pa=int(words[1])
            self.has[o_id]=i
            self.num[i]=o_id
            self.au[i]=pa
            i+=1
        fileHandle.close()
        fileHandle = open ( 'peo_year.txt')
        fileList=fileHandle.readlines()
        i=0
        for fileLine in fileList:
            words=fileLine.split(' ')
            self.peo[i]=int(words[1])
            i+=1
        fileHandle.close()
        fileHandle = open ( '/home/thinxer/aminer/cc2_s.txt' ) 
        fileList = fileHandle.readlines() 
        edge=0
        for fileLine in fileList: 
            words=fileLine.split(' ')
            x=int(words[0])
            idx=self.has[x]
            y=int(words[1])
            idy=self.has[y]
            z=int(words[2])
            edge+=1
            self.nex[edge]=self.first[idx]
            self.first[idx]=edge
            self.en[edge]=idy
            self.dis[edge]=z
        fileHandle.close() 

    def req(self, source, target):
        idx=self.has[source]
        idy=self.has[target]
        x=self.first[idx]
        while x!=-1:
            y=self.en[x]
            if (y==idy):
                return float(self.dis[x])
            x=self.nex[x]
        return 0.0
        """
        dui=[{} for x in range(1000)]
        idx=self.has[source]
        idy=self.has[target]
        for i in range(1000):
            dui[i]['node']=0
            dui[i]['wei']=0.0
            dui[i]['num']=0.0
        h=0
        t=1
        dui[t]['node']=idx
        dui[t]['wei']=0.0
        dui[t]['num']=0.0
        st=set()
        st.add(idx)
        tot=0
        while h<t:
            h+=1
            tot+=1
            now=dui[h]
            x=self.first[now['node']]
            y=self.en[x]
            if (tot>3):
                return 0.0
            while x!=-1:
                y=self.en[x]
                t+=1
                if t>=1000:
                    return 0.0
                dui[t]['node']=y
                dui[t]['num']=float(now['num']+1)
                dui[t]['wei']=float(now['wei']+self.dis[x])
                if y==idy:
                    return dui[t]['wei']/dui[t]['num']
                
                if (y in st == False):
                    t+=1
                    if t>=1000:
                        return 0.0
                    dui[t]['node']=y
                    dui[t]['num']=float(now['num']+1)
                    dui[t]['wei']=float(now['wei']+self.dis[x])
                    st.add(y)
                    if y==idy:
                        return dui[t]['wei']/dui[t]['num']
                
                x=self.nex[x]
        return 0.0
        """

    def ask(self,id):
        print ("start")
        a=int()
        a=id
        pub=self.client.influence_search_by_author("",a)
        result1=pub.influence
        result2=pub.influenced_by
        score=[{}for x in range(300)]
        for i in range(300):
            score[i]['score']=0
            score[i]['topic']=0
            score[i]['num']=0
        influence=[[]for x in range(300)]
        influenced_by=[[]for x in range(300)]
        num_influence=[0 for x in range(300)]
        num_influenced_by=[0 for x in range(300)]
        for i in result1:
            influence[i.topic].append(i)
            score[i.topic]['score']+=i.score
            score[i.topic]['topic']=i.topic
            num_influence[i.topic]+=1
        score.sort(key=lambda x:x['score'],reverse=True)
        for i in result2:
            influenced_by[i.topic].append(i)
            num_influenced_by[i.topic]+=1
        table_list=[]
        num=[0 for x in range(300)]
        peo=[[0 for x in range(300)] for y in range(300)]
        for i in range(0, 5):
            top_id=score[i]['topic']
            influence[top_id].sort(key=lambda x:x.score,reverse=True)
            influenced_by[top_id].sort(key=lambda x:x.score,reverse=True)
            num_1=min(5,num_influence[top_id])
            num_2=min(5,num_influenced_by[top_id])
            num[i]=num_1+num_2+1
            table={}
            table['nodes']=[]
            table['links']=[]
            peo[i][0]=a
            for j in range(0,num_1):
                peo[i][j+1]=influence[top_id][j].id
            for j in range(0,num_2):
                peo[i][j+num_1+1]=influenced_by[top_id][j].id
            dt=[0 for x in range(20)]
            gy=0.0
            tmpvalue=[[0.0 for y in range(20)]for x in range(20)]
            value=[[0.0 for y in range(20)]for x in range(20)]
            for j in range(0,num[i]):
                tmp={}
                tmp_id=peo[i][j]
                result=self.client.author_search_by_id("",[tmp_id])
                tmp['name']=result.entity[0].title
                dt[j]=result.entity[0].original_id
                tmp['group']=2
                table['nodes'].append(tmp)
            for j in range(0,num[i]):
                pub=self.client.influence_search_by_author("",peo[i][j])
                for k in range(0,num[i]):
                    if j==k:
                        continue
                    weight=0.0
                    for l in pub.influence:
                        if l.topic==top_id and l.id==peo[i][k]:
                            weight=l.score
                            break
                    if weight<0.1 and weight!=0:
                        weight=weight**(1.0/3)
                    value[j][k]=weight
                    if weight==0:
                        weight=self.req(dt[j], dt[k])
                        #print dt[j],dt[k]
                        if self.peo[self.has[dt[j]]]<self.peo[self.has[dt[k]]]:
                            tmpvalue[j][k]=weight
                        if weight>gy:
                            gy=weight
            for j in range(0,num[i]):
                weight=0.0
                for k in range(0,num[i]):
                    if j==k:
                        continue
                    tmp={}
                    tmp['source']=j
                    tmp['target']=k
                    weight=value[j][k]
                    if value[j][k]==0 and gy!=0:
                        weight=tmpvalue[j][k]/gy
                        if weight<0.1 and weight!=0:
                            weight=weight**(1.0/3)
                    tmp['value']=weight
                    table['links'].append(tmp)
            table_list.append(table)
        return table_list

#from saeclient import *
#client = SAEClient("tcp://10.1.1.111:40113")
#p=asker_table(client)
#print p.req(265966,575748)
