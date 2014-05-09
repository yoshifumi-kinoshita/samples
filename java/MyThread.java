
public class MyThread {

	public static void main(String[] args){
		for(int i=0; i<10; i++){
			Thread t = new Thread(new Runnable(){ public void run(){ try{ Thread.sleep(60000); } catch(Exception e){} } } );
			t.start();
		}
	}
}
