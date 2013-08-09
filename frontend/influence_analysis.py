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
		url="http://arnetminer.org/person/-/"
		url+=str(result.entity[0].original_id)
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
                url="http://arnetminer.org/person/-/"
		url+=str(result.entity[0].original_id)
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
        for item in pub_result.entity:
            year=item.stat[0].value
            pub[year].append(item)
            num[year]+=1
        trend=[]
        for i in range(1000,3000):
            if num[i]>0:
                pub[i].sort(key=lambda x:x.stat[2].value,reverse=True);
                tmp={}
                tmp['date']=str(i)
                tmp['value']=num[i]
                tmp['pap1']=pub[i][0].title
		result=self.client.author_search_by_id("",pub[i][0].related_entity[0].id)
		author = ', '.join([e.title for e in result.entity])
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
			author+=result.entity[j].title
			author+=', '
		    tmp['au2']=author
                    tmp['cit2']=pub[i][1].stat[2].value
                trend.append(tmp)
        return trend

class asker_table():
    
    def __init__(self, client):
        self.client = client

    def ask(self,id):
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
        score.sort(key=lambda x:x['score'],reverse=True);
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
            for j in range(0,num_1):
                peo[i][j]=influence[top_id][j].id
            for j in range(0,num_2):
                peo[i][j+num_1]=influenced_by[top_id][j].id
            peo[i][num[i]-1]=a
            st=set()
            for j in range(0,num[i]):
                tmp={}
                tmp_id=peo[i][j]
                st.add(peo[i][j])
                result=self.client.author_search_by_id("",[tmp_id])
                tmp['name']=result.entity[0].title
                tmp['group']=2
                table['nodes'].append(tmp)
            #print st
            #res_in=[[]for x in range(100)]
            #for j in range(0,num[i]):
                #res_in[j]=self.client.influence_search_by_author("",peo[i][j]).influence
            for j in range(0,num[i]):
                pub=self.client.influence_search_by_author("",peo[i][j])
                tmp={}
                weight=0.0       
                for l in pub.influence:
                    if l.topic==top_id and (l.id in st)==True:
                        weight=l.score
                        k=l.id
                        break      
                #if weight==0:
                   # for l in pub.influence:
                        #pub2=self.client.influence_search_by_author("",l.id)
                        #flag=0
                        #for q in pub2.influence:
                           # if q.topic==top_id and (q.id in st)==True:
                           #     weight=q.score
                          #      flag=1
                        #        break
                      #  if flag==1:
                      #      break
                        """
                        if l.topic==top_id:
                            pub2=self.client.influence_search_by_author("",l.id)
                            flag=0
                            for q in pub2.influence:
                                if q.topic==top_id and (q.id in st)==True:
                                    weight=q.score
                                    flag=1
                                    break
                            if flag==1:
                                break
                        """
                if weight==0:
                    continue
                weight=weight**(1.0/3)
                if weight<0.5:
                    weight+=0.3
                tmp['source']=j
                for q in range(0,num[i]):
                    if peo[i][q]==k:
                        res=q
                        break
                tmp['target']=q
                tmp['value']=weight
                table['links'].append(tmp)
            table_list.append(table)
        return table_list
    
#from saeclient import *
#client = SAEClient("tcp://10.1.1.111:40114")
#p=asker_table(client)
#print p.ask(744714)
